#include <QtDebug>

#include "engine/positionscratchcontroller.h"
#include "mathstuff.h"

#ifdef _MSC_VER
#include <float.h>  // for _finite() on VC++
#define isinf(x) (!_finite(x))
#endif

class VelocityController {
  public:
    VelocityController() {
        m_p = 0.0;
        m_i = 0.0;
        m_d = 0.0;
        m_target_position = 0;
        m_last_error = 0.0;
        m_last_time = 0.0;
        m_error_sum = 0.0;
        m_last_velocity = 0.0;
    }

    void setPID(double p, double i, double d) {
        m_p = p;
        m_i = i;
        m_d = d;
    }

    void reset(double position, double time, double target_position) {
        m_target_position = target_position;
        m_last_error = m_target_position - position;
        m_last_time = time;
        m_error_sum = 0.0;
        m_last_velocity = 0.0;
    }

    void setTarget(double target_position) {
        m_target_position = target_position;
    }

    double observation(double position, double time) {
        // limit dt to 100 microseconds to prevent blowup (nobody has latency
        // that low so this shouldn't happen in practice).
        double dt = math_max(time - m_last_time, 0.000100);

        const double error = m_target_position - position;

        // Calculate integral component of PID
        m_error_sum += error * dt;

        // Calculate differential component of PID. Positive if we're getting
        // worse, negative if we're getting closer.
        double error_change = (error - m_last_error) / dt;

        // Indicator that can possibly tell if we've gone unstable and are
        // oscillating around the target.
        //const bool error_flip = (error < 0 && m_last_error > 0) || (error > 0 && m_last_error < 0);

        // Protect against silly error_change values.
        if (isnan(error_change) || isinf(error_change))
            error_change = 0.0;

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

        m_last_velocity = output;
        m_last_error = error;
        return output;
    }

    double getTarget() {
        return m_target_position;
    }

  private:
    double m_target_position;
    double m_last_error;
    double m_last_time;
    double m_last_velocity;
    double m_error_sum;
    double m_p, m_i, m_d;
};

PositionScratchController::PositionScratchController(const char* pGroup)
        : m_group(pGroup) {
    m_pScratchEnable = new ControlObject(ConfigKey(pGroup, "scratch_position_enable"));
    m_pScratchPosition = new ControlObject(ConfigKey(pGroup, "scratch_position"));
    m_pMasterSampleRate = ControlObject::getControl(ConfigKey("[Master]", "samplerate"));
    m_pScratchControllerP = new ControlObject(ConfigKey(pGroup, "scratch_constant_p"));
    m_pScratchControllerI = new ControlObject(ConfigKey(pGroup, "scratch_constant_i"));
    m_pScratchControllerD = new ControlObject(ConfigKey(pGroup, "scratch_constant_d"));
    m_pVelocityController = new VelocityController();
    m_bScratching = false;
    m_dScratchTime = 0;
    m_bEnableInertia = false;
    m_dRate = 0.;
    m_pScratchControllerP->set(0.0002);
    m_pScratchControllerI->set(0.0);
    m_pScratchControllerD->set(0.0);

    //m_pVelocityController->setPID(0.2, 1.0, 5.0);
    //m_pVelocityController->setPID(0.1, 0.0, 5.0);
    //m_pVelocityController->setPID(0.0001, 0.0, 0.00);
}

PositionScratchController::~PositionScratchController() {
    delete m_pScratchEnable;
    delete m_pScratchPosition;
    delete m_pVelocityController;
    delete m_pScratchControllerP;
    delete m_pScratchControllerI;
    delete m_pScratchControllerD;
}

void PositionScratchController::process(double currentSample, bool paused, int iBufferSize) {
    bool scratchEnable = m_pScratchEnable->get() != 0;
    double scratchPosition = m_pScratchPosition->get();
    m_pVelocityController->setPID(m_pScratchControllerP->get(),
                                  m_pScratchControllerI->get(),
                                  m_pScratchControllerD->get());

    // The rate threshold above which disabling position scratching will enable
    // an 'inertia' mode.
    const double kThrowThreshold = 2.5;

    // If we're playing, then do not decay rate below 1. If we're not playing,
    // then we want to decay all the way down to below 0.01
    const double kDecayThreshold = paused ? 0.01 : 1.0;
    // Max velocity we would like to stop in a given time period.
    const double kMaxVelocity = 100;
    // Seconds to stop a throw at the max velocity.
    const double kTimeToStop = 2.0;

    // The latency or time difference between process calls.
    const double dt = static_cast<double>(iBufferSize) / m_pMasterSampleRate->get();

    // We calculate the exponential decay constant based on the above
    // constants. Roughly we backsolve what the decay should be if we want to
    // stop a throw of max velocity kMaxVelocity in kTimeToStop seconds. Here is
    // the derivation:
    // kMaxVelocity * alpha ^ (# callbacks to stop in) = kDecayThreshold
    // # callbacks = kTimeToStop / dt
    // alpha = (kDecayThreshold / kMaxVelocity) ^ (dt / kTimeToStop)
    const double kExponentialDecay = pow(kDecayThreshold / kMaxVelocity, dt / kTimeToStop);

    if (m_bScratching) {
        if (scratchEnable || m_bEnableInertia) {
            // We were previously in scratch mode and are still in scratch mode
            // OR we are in inertia mode.

            if (scratchEnable) {
                // If we're scratching, clear the inertia flag. This case should
                // have been caught by the 'enable' case below, but just to make
                // sure.
                m_bEnableInertia = false;

                // Increment processing timer by time delta.
                m_dScratchTime += dt;

                // Set the scratch target to the current set position
                m_pVelocityController->setTarget(scratchPosition);

                // Measure the total distance travelled since last frame and add
                // it to the running total.
                m_dPositionDeltaSum += (currentSample - m_dLastPlaypos);

                m_dRate = m_pVelocityController->observation(
                    m_dPositionDeltaSum, m_dScratchTime);
                //qDebug() << "continue" << m_dRate << iBufferSize;
            } else {
                // If we got here then we're not scratching and we're in inertia
                // mode. Take the previous rate that was set and apply a
                // deceleration.
                m_dRate *= kExponentialDecay;

                // If the rate has decayed below the threshold, then leave
                // inertia mode.
                if (fabs(m_dRate) <= kDecayThreshold) {
                    m_bEnableInertia = false;
                }
            }
        } else {
            // We were previously in scratch mode and are no longer in scratch
            // mode. Disable everything, or optionally enable inertia mode if
            // the previous rate was high enough to count as a 'throw'
            if (fabs(m_dRate) > kThrowThreshold) {
                m_bEnableInertia = true;
            } else {
                m_dRate = 0;
                m_bScratching = false;
                m_dScratchTime = 0;
            }
            //qDebug() << "disable";
        }
    } else {
        if (scratchEnable) {
            // We were not previously in scratch mode but now are in scratch
            // mode. Enable scratching.
            m_bScratching = true;
            m_bEnableInertia = false;
            m_dPositionDeltaSum = 0;
            m_dScratchTime = 0;
            m_pVelocityController->reset(0, m_dScratchTime, scratchPosition);
            m_dRate = m_pVelocityController->observation(0, m_dScratchTime);
            //qDebug() << "enable" << m_dRate << currentSample;
        } else {
            // We were not previously in scratch mode are still not in scratch
            // mode. Do nothing
        }
    }
    m_dLastPlaypos = currentSample;
}

bool PositionScratchController::isEnabled() {
    return m_bScratching;
}

double PositionScratchController::getRate() {
    return m_dRate;
}

void PositionScratchController::notifySeek(double currentSample) {
    m_dLastPlaypos = currentSample;
}
