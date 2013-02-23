#include <QtDebug>

#include "engine/positionscratchcontroller.h"
#include "engine/enginebufferscale.h" // for MIN_SEEK_SPEED
#include "mathstuff.h"

#ifdef _MSC_VER
#include <float.h>  // for _finite() on VC++
#define isinf(x) (!_finite(x))
#endif

class VelocityController {
  public:
    VelocityController()
        : m_last_error(0.0),
          m_error_sum(0.0),
          m_p(0.0),
          m_i(0.0),
          m_d(0.0) {
    }

    void setPID(double p, double i, double d) {
        m_p = p;
        m_i = i;
        m_d = d;
    }

    void reset() {
        m_last_error = 0;
        m_error_sum = 0.0;
    }

    double observation(double position, double target_position, double dt) {
        Q_UNUSED(dt) // Since the controller runs with constant sample rate
                     // we don't have to deal with dt inside the controller

        const double error = target_position - position;

        // Calculate integral component of PID
        // In case of error too small then stop integration
        if (abs(error) > 0.1) {
            m_error_sum += error;
        }

        // Calculate differential component of PID. Positive if we're getting
        // worse, negative if we're getting closer.
        double error_change = (error - m_last_error);

        // qDebug() << "target:" << m_target_position << "position:" << position
        //          << "error:" << error << "change:" << error_change << "sum:" << m_error_sum;

        // Main PID calculation
        double output = m_p * error + m_i * m_error_sum + m_d * error_change;

        // Try to stabilize us if we're close to the target. Otherwise we might
        // overshoot and oscillate.
        //if (fabs(error) < m_samples_per_buffer) {
            //double percent_remaining = error / m_samples_per_buffer;
            //// Apply exponential decay to try and stop the stuttering.
            //double decay = (1.0 - pow(2, -fabs(percent_remaining)));
            //output = percent_remaining * decay;
            //qDebug() << "clamp decay" << decay << "output" << output;
        //}

        m_last_error = error;
        return output;
    }

  private:
    double m_last_error;
    double m_error_sum;
    double m_p, m_i, m_d;
};

PositionScratchController::PositionScratchController(const char* pGroup)
    : m_group(pGroup),
      m_bScratching(false),
      m_bEnableInertia(false),
      m_dLastPlaypos(0),
      m_dPositionDeltaSum(0),
      m_dStartScratchPosition(0),
      m_dRate(0) {
    m_pScratchEnable = new ControlObject(ConfigKey(pGroup, "scratch_position_enable"));
    m_pScratchPosition = new ControlObject(ConfigKey(pGroup, "scratch_position"));
    m_pMasterSampleRate = ControlObject::getControl(ConfigKey("[Master]", "samplerate"));
    m_pVelocityController = new VelocityController();

    //m_pVelocityController->setPID(0.2, 1.0, 5.0);
    //m_pVelocityController->setPID(0.1, 0.0, 5.0);
    //m_pVelocityController->setPID(0.0001, 0.0, 0.00);
}

PositionScratchController::~PositionScratchController() {
    delete m_pScratchEnable;
    delete m_pScratchPosition;
    delete m_pVelocityController;
}


void PositionScratchController::process(double currentSample, bool paused,
        int iBufferSize, double baserate) {
    bool scratchEnable = m_pScratchEnable->get() != 0;

   	if (!m_bScratching && scratchEnable) {
        // We were not previously in scratch mode are still not in scratch
        // mode. Do nothing
    }

    double scratchPosition = m_pScratchPosition->get();
    
    // The latency or time difference between process calls.
    const double dt = static_cast<double>(iBufferSize)
            / m_pMasterSampleRate->get() / 2;

    // Tweak PID controller for different latencies
    double p;
    double i;
    double d;
    if (dt > 0.015) {
        // High latency
        p = 0.3;
        i = 0;
        d = 0;
    } else {
        // Low latency
        p = 17 * dt; // ~ 0.2 for 11,6 ms
        i = 0;
        d = 0;
    }
    m_pVelocityController->setPID(p, i, d);

    if (m_bScratching) {
        if (m_bEnableInertia) {
            // If we got here then we're not scratching and we're in inertia
            // mode. Take the previous rate that was set and apply a
            // deceleration.

            // If we're playing, then do not decay rate below 1. If we're not playing,
            // then we want to decay all the way down to below 0.01
            const double kDecayThreshold = paused ? 0.01 : 1.0;

            // Max velocity we would like to stop in a given time period.
            const double kMaxVelocity = 100;
            // Seconds to stop a throw at the max velocity.
            const double kTimeToStop = 1.0;

            // We calculate the exponential decay constant based on the above
            // constants. Roughly we backsolve what the decay should be if we want to
            // stop a throw of max velocity kMaxVelocity in kTimeToStop seconds. Here is
            // the derivation:
            // kMaxVelocity * alpha ^ (# callbacks to stop in) = kDecayThreshold
            // # callbacks = kTimeToStop / dt
            // alpha = (kDecayThreshold / kMaxVelocity) ^ (dt / kTimeToStop)
            const double kExponentialDecay = pow(kDecayThreshold / kMaxVelocity, dt / kTimeToStop);

            m_dRate *= kExponentialDecay;

            // If the rate has decayed below the threshold, or scartching is 
            // reanabled then leave inertia mode.
            if (fabs(m_dRate) < kDecayThreshold || scratchEnable) {
                m_bEnableInertia = false;
                m_bScratching = false;
            }        
        } else if (scratchEnable) {
            // If we're scratching, clear the inertia flag. This case should
            // have been caught by the 'enable' case below, but just to make
            // sure.
            m_bEnableInertia = false;

            // Set the scratch target to the current set position
            // and normalize to one buffer
            double targetDelta = (scratchPosition - m_dStartScratchPosition) /
                    (iBufferSize * baserate);

            // Measure the total distance traveled since last frame and add
            // it to the running total. This is required to scratch within loop
            // boundaries. Normalize to one buffer
            m_dPositionDeltaSum += (currentSample - m_dLastPlaypos) /
                    (iBufferSize * baserate);

            m_dRate = m_pVelocityController->observation(
                    m_dPositionDeltaSum, targetDelta, dt);
            if (m_dRate < MIN_SEEK_SPEED && m_dRate > -MIN_SEEK_SPEED) {
                // we cannot get closer
                m_dRate = 0;
            }
            //qDebug() << m_dRate << scratchPosition << targetDelta - m_dPositionDeltaSum << targetDelta << m_dPositionDeltaSum << dt;
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
			m_dRate = 0; 
			m_dPositionDeltaSum = 0;    
            m_dStartScratchPosition = scratchPosition;
            m_pVelocityController->reset();
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
