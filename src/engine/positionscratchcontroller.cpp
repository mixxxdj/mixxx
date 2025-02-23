#include "engine/positionscratchcontroller.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "engine/bufferscalers/enginebufferscale.h" // for MIN_SEEK_SPEED
#include "moc_positionscratchcontroller.cpp"
#include "preferences/configobject.h" // for ConfigKey
#include "util/math.h"
#include "util/time.h"

class VelocityController {
  public:
    VelocityController()
            : m_last_error(0.0),
              m_p(0.0),
              m_d(0.0) {
    }
    void setPD(double p, double d) {
        m_p = p;
        m_d = d;
    }

    void reset(double last_error) {
        m_last_error = last_error;
    }

    double observation(double error) {
        // Main PD calculation
        m_last_error = m_p * error + m_d * (error - m_last_error);
        return m_last_error;
    }

  private:
    double m_last_error;
    double m_p, m_d;
};

class RateIIFilter {
  public:
    RateIIFilter()
            : m_factor(1.0),
              m_last_rate(0.0) {
    }

    void setFactor(double factor) {
        m_factor = factor;
    }

    void reset(double last_rate) {
        m_last_rate = last_rate;
    }

    double filter(double rate) {
        if (fabs(rate) - fabs(m_last_rate) > -0.1) {
            m_last_rate = m_last_rate * (1 - m_factor) + rate * m_factor;
        } else {
            // do not filter strong decelerations to avoid overshooting
            m_last_rate = rate;
        }
        return m_last_rate;
    }

  private:
    double m_factor;
    double m_last_rate;
};

namespace {

constexpr double kDefaultSampleInterval = 0.016;
// The max wait time when no new position has been set
constexpr double kMoveDelayMax = 0.04;
// The rate threshold above which disabling position scratching will enable
// an 'inertia' mode.
constexpr double kThrowThreshold = 2.5;
// Max velocity we would like to stop in a given time period.
constexpr double kMaxVelocity = 100;
// Seconds to stop a throw at the max velocity.
constexpr double kTimeToStop = 1.0;

} // anonymous namespace

PositionScratchController::PositionScratchController(const QString& group)
        : m_group(group),
          m_pScratchEnable(std::make_unique<ControlObject>(
                  ConfigKey(group, QStringLiteral("scratch_position_enable")))),
          m_pScratchPos(std::make_unique<ControlObject>(
                  ConfigKey(group, QStringLiteral("scratch_position")))),
          m_pMainSampleRate(std::make_unique<ControlProxy>(
                  ConfigKey(QStringLiteral("[App]"), QStringLiteral("samplerate")))),
          m_pVelocityController(std::make_unique<VelocityController>()),
          m_pRateIIFilter(std::make_unique<RateIIFilter>()),
          m_isScratching(false),
          m_inertiaEnabled(false),
          m_prevSamplePos(0),
          m_samplePosDeltaSum(0),
          m_scratchTargetDelta(0),
          m_scratchStartPos(0),
          m_rate(0),
          m_moveDelay(0),
          m_mouseSampleTime(0),
          m_bufferSize(-1), // ?
          m_dt(1),
          m_callsPerDt(1),
          m_p(1),
          m_d(1),
          m_f(0.4) {
    m_pMainSampleRate->connectValueChanged(this,
            &PositionScratchController::slotUpdateFilterParameters);
}

PositionScratchController::~PositionScratchController() {
}

void PositionScratchController::slotUpdateFilterParameters(double sampleRate) {
    // The latency or time difference between process calls.
    m_dt = static_cast<double>(m_bufferSize) / sampleRate / 2;

    // Sample Mouse with fixed timing intervals to iron out significant jitters
    // that are added on the way from mouse to engine thread
    // Normally the Mouse is sampled every 8 ms so with this 16 ms window we
    // have 0 ... 3 samples. The remaining jitter is ironed by the following IIR
    // lowpass filter
    m_callsPerDt = static_cast<int>(ceil(kDefaultSampleInterval / m_dt));

    // Tweak PD controller for different latencies
    m_p = 0.3;
    m_d = m_p / -2;
    m_f = 0.4;
    if (m_dt > kDefaultSampleInterval * 2) {
        m_f = 1;
    }

    m_pVelocityController->setPD(m_p, m_d);
    m_pRateIIFilter->setFactor(m_f);
}

void PositionScratchController::process(double currentSamplePos,
        double releaseRate,
        std::size_t bufferSize,
        double baseSampleRate,
        int wrappedAround,
        mixxx::audio::FramePos trigger,
        mixxx::audio::FramePos target) {
    bool scratchEnable = m_pScratchEnable->get() != 0;

    if (!m_isScratching && !scratchEnable) {
        // We were not previously in scratch mode are still not in scratch
        // mode. Do nothing
        return;
    }

    if (bufferSize != m_bufferSize) {
        m_bufferSize = bufferSize;
        slotUpdateFilterParameters(m_pMainSampleRate->get());
    }

    double scratchPosition = 0;
    m_mouseSampleTime += m_dt;
    if (m_mouseSampleTime >= kDefaultSampleInterval || !m_isScratching) {
        scratchPosition = m_pScratchPos->get();
        m_mouseSampleTime = 0;
    }

    if (m_isScratching) {
        if (m_inertiaEnabled) {
            // If we got here then we're not scratching and we're in inertia
            // mode. Take the previous rate that was set and apply a
            // deceleration.

            // If we're playing, then do not decay rate below 1. If we're not playing,
            // then we want to decay all the way down to below 0.01
            double decayThreshold = fabs(releaseRate);
            if (decayThreshold < MIN_SEEK_SPEED) {
                decayThreshold = MIN_SEEK_SPEED;
            }

            // We calculate the exponential decay constant based on the above
            // constants. Roughly we backsolve what the decay should be if we want to
            // stop a throw of max velocity kMaxVelocity in kTimeToStop seconds. Here is
            // the derivation:
            // kMaxVelocity * alpha ^ (# callbacks to stop in) = decayThreshold
            // # callbacks = kTimeToStop / m_dt
            // alpha = (decayThreshold / kMaxVelocity) ^ (m_dt / kTimeToStop)
            const double kExponentialDecay = pow(decayThreshold / kMaxVelocity, m_dt / kTimeToStop);

            m_rate *= kExponentialDecay;

            // If the rate has decayed below the threshold, or scratching is
            // re-enabled then leave inertia mode.
            if (fabs(m_rate) < decayThreshold || scratchEnable) {
                m_inertiaEnabled = false;
                m_isScratching = false;
            }
            // qDebug() << m_rate << kExponentialDecay << m_dt;
        } else if (scratchEnable) {
            // If we're scratching, clear the inertia flag. This case should
            // have been caught by the 'enable' case below, but just to make
            // sure.
            m_inertiaEnabled = false;

            double sampleDelta = 0.0;
            if (wrappedAround > 0) {
                // If we wrapped around calculate the virtual position like if
                // we are not looping, i.e. sum up diffs from loop start/end and
                // loop length for each wrap-aound (necessary if the buffer is
                // longer than the loop, e.g. when looping at high rates / with short loops.
                // This avoids high rate and infinite wrap-around scratching
                // even after mouse was stopped.
                double triggerPos = trigger.toEngineSamplePos();
                double targetPos = target.toEngineSamplePos();
                bool reverse = triggerPos < targetPos;
                double loopLength = reverse ? -1 * (targetPos - triggerPos)
                                            : triggerPos - targetPos;
                if (wrappedAround > 2) {
                    sampleDelta = (wrappedAround - 1) * loopLength;
                }
                sampleDelta +=
                        (triggerPos - m_prevSamplePos) +
                        (currentSamplePos - targetPos);
            } else {
                sampleDelta = currentSamplePos - m_prevSamplePos;
            }

            // Measure the total distance traveled since last frame and add
            // it to the running total. This is required to scratch within loop
            // boundaries. And normalize to one buffer
            m_samplePosDeltaSum += (sampleDelta) / (bufferSize * baseSampleRate);

            // If we may have a new position, calculate scratch parameters and
            // eventually the new rate.
            // Else, continue with the last rate.
            if (m_mouseSampleTime == 0) {
                // Set the scratch target to the current set position
                // and normalize to one buffer
                double scratchTargetDelta = (scratchPosition - m_scratchStartPos) /
                        (bufferSize * baseSampleRate);

                bool calcRate = true;

                if (m_scratchTargetDelta == scratchTargetDelta) {
                    // We get here if the next mouse position is delayed, the mouse
                    // is stopped or moves very slowly. Since we don't know the case
                    // we assume delayed mouse updates for 40 ms.
                    // TODO Make threshold configurable for controller use?
                    m_moveDelay += m_dt * m_callsPerDt;
                    if (m_moveDelay < kMoveDelayMax) {
                        // Assume a missing Mouse Update and continue with the
                        // previously calculated rate.
                        calcRate = false;
                    } else {
                        // Mouse has stopped
                        m_pVelocityController->setPD(m_p, 0);
                        if (scratchTargetDelta == 0) {
                            // Mouse was not moved at all
                            // Stop immediately by restarting the controller
                            // in stopped mode
                            m_pVelocityController->reset(0);
                            m_pRateIIFilter->reset(0);
                            m_samplePosDeltaSum = 0;
                        }
                    }
                } else {
                    m_moveDelay = 0;
                    m_scratchTargetDelta = scratchTargetDelta;
                }

                if (calcRate) {
                    double ctrlError = m_pRateIIFilter->filter(
                            scratchTargetDelta - m_samplePosDeltaSum);
                    m_rate = m_pVelocityController->observation(ctrlError);
                    m_rate /= ceil(kDefaultSampleInterval / m_dt);
                    // Note: The following SoundTouch changes the also rate by a ramp
                    // This looks like average of the new and the old rate independent
                    // from m_dt. Ramping is disabled when direction changes or rate = 0;
                    // (determined experimentally)
                    if (fabs(m_rate) < MIN_SEEK_SPEED) {
                        // we cannot get closer
                        m_rate = 0;
                    }
                }

                // qDebug() << m_rate << scratchTargetDelta << m_samplePosDeltaSum << m_dt;
            }
        } else {
            // We were previously in scratch mode and are no longer in scratch
            // mode. Disable everything, or optionally enable inertia mode if
            // the previous rate was high enough to count as a 'throw'
            if (fabs(m_rate) > kThrowThreshold) {
                m_inertiaEnabled = true;
            } else {
                m_isScratching = false;
            }
            //qDebug() << "disable";
        }
    } else if (scratchEnable) {
        // We were not previously in scratch mode but now are in scratch
        // mode. Enable scratching.
        m_isScratching = true;
        m_inertiaEnabled = false;
        m_moveDelay = 0;
        // Set up initial values, in a way that the system is settled
        m_rate = releaseRate;
        m_samplePosDeltaSum = -(releaseRate / m_p) *
                m_callsPerDt; // Set to the remaining error of a p controller
        m_pVelocityController->reset(-m_samplePosDeltaSum);
        m_pRateIIFilter->reset(-m_samplePosDeltaSum);
        m_scratchStartPos = scratchPosition;
        // qDebug() << "scratchEnable()" << currentSamplePos;
    }
    m_prevSamplePos = currentSamplePos;
}

void PositionScratchController::notifySeek(mixxx::audio::FramePos position) {
    DEBUG_ASSERT(position.isValid());
    // scratching continues after seek due to calculating the relative distance traveled
    // in m_samplePosDeltaSum
    m_prevSamplePos = position.toEngineSamplePos();
}
