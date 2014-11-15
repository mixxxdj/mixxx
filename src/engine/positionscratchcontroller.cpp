#include <QtDebug>

#include "engine/positionscratchcontroller.h"
#include "engine/enginebufferscale.h" // for MIN_SEEK_SPEED
#include "util/math.h"

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
        }  else {
            // do not filter strong decelerations to avoid overshooting
            m_last_rate = rate;
        }
        return m_last_rate;
    }

  private:
    double m_factor;
    double m_last_rate;
};

PositionScratchController::PositionScratchController(QString group)
    : m_group(group),
      m_bScratching(false),
      m_bEnableInertia(false),
      m_dLastPlaypos(0),
      m_dPositionDeltaSum(0),
      m_dTargetDelta(0),
      m_dStartScratchPosition(0),
      m_dRate(0),
      m_dMoveDelay(0),
      m_dMouseSampeTime(0) {
    m_pScratchEnable = new ControlObject(ConfigKey(group, "scratch_position_enable"));
    m_pScratchPosition = new ControlObject(ConfigKey(group, "scratch_position"));
    m_pMasterSampleRate = ControlObject::getControl(ConfigKey("[Master]", "samplerate"));
    m_pVelocityController = new VelocityController();
    m_pRateIIFilter = new RateIIFilter;
}

PositionScratchController::~PositionScratchController() {
    delete m_pRateIIFilter;
    delete m_pVelocityController;
    delete m_pScratchPosition;
    delete m_pScratchEnable;
}

//volatile double _p = 0.3;
//volatile double _d = -0.15;
//volatile double _f = 0.5;

void PositionScratchController::process(double currentSample, double releaseRate,
        int iBufferSize, double baserate) {
    bool scratchEnable = m_pScratchEnable->get() != 0;

    if (!m_bScratching && !scratchEnable) {
        // We were not previously in scratch mode are still not in scratch
        // mode. Do nothing
        return;
    }

    // The latency or time difference between process calls.
    const double dt = static_cast<double>(iBufferSize)
            / m_pMasterSampleRate->get() / 2;

    // Sample Mouse with fixed timing intervals to iron out significant jitters
    // that are added on the way from mouse to engine thread
    // Normaly the Mouse is sampled every 8 ms so with this 16 ms window we
    // have 0 ... 3 samples. The remaining jitter is ironed by the following IIR
    // lowpass filter
    const double m_dMouseSampeIntervall = 0.016;
    const int callsPerDt = ceil(m_dMouseSampeIntervall/dt);
    double scratchPosition = 0;
    m_dMouseSampeTime += dt;
    if (m_dMouseSampeTime >= m_dMouseSampeIntervall || !m_bScratching) {
        scratchPosition = m_pScratchPosition->get();
        m_dMouseSampeTime = 0;
    }

    // Tweak PD controller for different latencies
    double p = 0.3;
    double d = p/-2;
    double f = 0.4;
    if (dt > m_dMouseSampeIntervall * 2) {
        f = 1;
    }
    m_pVelocityController->setPD(p, d);
    m_pRateIIFilter->setFactor(f);
    //m_pVelocityController->setPID(_p, _i, _d);
    //m_pMouseRateIIFilter->setFactor(_f);

    if (m_bScratching) {
        if (m_bEnableInertia) {
            // If we got here then we're not scratching and we're in inertia
            // mode. Take the previous rate that was set and apply a
            // deceleration.

            // If we're playing, then do not decay rate below 1. If we're not playing,
            // then we want to decay all the way down to below 0.01
            double decayThreshold = fabs(releaseRate);
            if (decayThreshold < MIN_SEEK_SPEED) {
                decayThreshold = MIN_SEEK_SPEED;
            }

            // Max velocity we would like to stop in a given time period.
            const double kMaxVelocity = 100;
            // Seconds to stop a throw at the max velocity.
            const double kTimeToStop = 1.0;

            // We calculate the exponential decay constant based on the above
            // constants. Roughly we backsolve what the decay should be if we want to
            // stop a throw of max velocity kMaxVelocity in kTimeToStop seconds. Here is
            // the derivation:
            // kMaxVelocity * alpha ^ (# callbacks to stop in) = decayThreshold
            // # callbacks = kTimeToStop / dt
            // alpha = (decayThreshold / kMaxVelocity) ^ (dt / kTimeToStop)
            const double kExponentialDecay = pow(decayThreshold / kMaxVelocity, dt / kTimeToStop);

            m_dRate *= kExponentialDecay;

            // If the rate has decayed below the threshold, or scratching is
            // re-enabled then leave inertia mode.
            if (fabs(m_dRate) < decayThreshold || scratchEnable) {
                m_bEnableInertia = false;
                m_bScratching = false;
            }
            //qDebug() << m_dRate << kExponentialDecay << dt;
        } else if (scratchEnable) {
            // If we're scratching, clear the inertia flag. This case should
            // have been caught by the 'enable' case below, but just to make
            // sure.
            m_bEnableInertia = false;

            // Measure the total distance traveled since last frame and add
            // it to the running total. This is required to scratch within loop
            // boundaries. And normalize to one buffer
            m_dPositionDeltaSum += (currentSample - m_dLastPlaypos) /
                    (iBufferSize * baserate);

            // Continue with the last rate if we do not have a new
            // Mouse position
            if (m_dMouseSampeTime ==  0) {

                // Set the scratch target to the current set position
                // and normalize to one buffer
                double targetDelta = (scratchPosition - m_dStartScratchPosition) /
                        (iBufferSize * baserate);

                bool calcRate = true;

                if (m_dTargetDelta == targetDelta) {
                    // we get here, if the next mouse position is delayed
                    // the mouse is stopped or moves slow. Since we don't know the case
                    // we assume delayed mouse updates for 40 ms
                    m_dMoveDelay += dt * callsPerDt;
                    if (m_dMoveDelay < 0.04) {
                        // Assume a missing Mouse Update and continue with the
                        // previously calculated rate.
                        calcRate = false;
                    } else {
                        // Mouse has stopped
                        m_pVelocityController->setPD(p, 0);
                        if (targetDelta == 0) {
                            // Mouse was not moved at all
                            // Stop immediately by restarting the controller
                            // in stopped mode
                            m_pVelocityController->reset(0);
                            m_pRateIIFilter->reset(0);
                            m_dPositionDeltaSum = 0;
                        }
                    }
                } else {
                    m_dMoveDelay = 0;
                    m_dTargetDelta = targetDelta;
                }

                if (calcRate) {
                    double ctrlError = m_pRateIIFilter->filter(targetDelta - m_dPositionDeltaSum);
                    m_dRate = m_pVelocityController->observation(ctrlError);
                    m_dRate /= ceil(m_dMouseSampeIntervall/dt);
                    // Note: The following SoundTouch changes the also rate by a ramp
                    // This looks like average of the new and the old rate independent
                    // from dt. Ramping is disabled when direction changes or rate = 0;
                    // (determined experimentally)
                    if (fabs(m_dRate) < MIN_SEEK_SPEED) {
                        // we cannot get closer
                        m_dRate = 0;
                    }
                }

                //qDebug() << m_dRate << targetDelta << m_dPositionDeltaSum << dt;
            }
        } else {
            // We were previously in scratch mode and are no longer in scratch
            // mode. Disable everything, or optionally enable inertia mode if
            // the previous rate was high enough to count as a 'throw'

            // The rate threshold above which disabling position scratching will enable
            // an 'inertia' mode.
            const double kThrowThreshold = 2.5;

            if (fabs(m_dRate) > kThrowThreshold) {
                m_bEnableInertia = true;
            } else {
                m_bScratching = false;
            }
            //qDebug() << "disable";
        }
    } else if (scratchEnable) {
            // We were not previously in scratch mode but now are in scratch
            // mode. Enable scratching.
            m_bScratching = true;
            m_bEnableInertia = false;
            m_dMoveDelay = 0;
            // Set up initial values, in a way that the system is settled
            m_dRate = releaseRate;
            m_dPositionDeltaSum = -(releaseRate / p) * callsPerDt; // Set to the remaining error of a p controller
            m_pVelocityController->reset(-m_dPositionDeltaSum);
            m_pRateIIFilter->reset(-m_dPositionDeltaSum);
            m_dStartScratchPosition = scratchPosition;
            //qDebug() << "scratchEnable()" << currentSample;
    }
    m_dLastPlaypos = currentSample;
}

bool PositionScratchController::isEnabled() {
    // return true only if m_dRate is valid.
    return m_bScratching;
}

double PositionScratchController::getRate() {
    return m_dRate;
}

void PositionScratchController::notifySeek(double currentSample) {
    // scratching continues after seek due to calculating the relative distance traveled
    // in m_dPositionDeltaSum
    m_dLastPlaypos = currentSample;
}
