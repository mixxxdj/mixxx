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
        m_samples_per_buffer = 0;
        m_last_velocity = 0.0;
    }

    void setPID(double p, double i, double d) {
        m_p = p;
        m_i = i;
        m_d = d;
    }

    void setSamplesPerBuffer(int iSamplesPerBuffer) {
        m_samples_per_buffer = iSamplesPerBuffer;
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
        double dt = math_max(time - m_last_time, .001); // limit dt blowup
        dt = 1; // TODO(rryan) all time stuff disabled for now

        const double error = m_target_position - position;

        // Calculate integral component of PID
        m_error_sum += error * dt;
        // WTF ok..
        m_error_sum = m_last_error + error;

        // Calculate differential component of PID. Positive if we're getting
        // worse, negative if we're getting closer.
        double error_change = (error - m_last_error) / dt;

        // Indicator that can possibly tell if we've gone unstable and are
        // oscillating around the target.
        const bool error_flip = (error < 0 && m_last_error > 0) || (error > 0 && m_last_error < 0);

        if (isnan(error_change) || isinf(error_change))
            error_change = 0.0;

        // qDebug() << "target:" << m_target_position << "position:" << position
        //          << "error:" << error << "change:" << error_change << "sum:" << m_error_sum;

        // Main PID calculation
        double output = m_p * error + m_i * m_error_sum + m_d * error_change;

        // Divide by samples per buffer to get a rate normalized for producing
        // m_samples_per_buffer samples for a value of 1.0
        output /= m_samples_per_buffer;

        // Try to stabilize us if we're close to the target. Otherwise we might
        // overshoot and oscillate.
        if (fabs(error) < m_samples_per_buffer) {
            double percent_remaining = error / m_samples_per_buffer;
            // Apply exponential decay to try and stop the stuttering.
            double decay = (1.0 - pow(2, -fabs(percent_remaining)));
            //output = percent_remaining * decay;
            //qDebug() << "clamp decay" << decay << "output" << output;
        }

        m_last_velocity = output;
        m_last_error = error;
        return output;
    }

    double getTarget() {
        return m_target_position;
    }

  private:
    double m_samples_per_buffer;
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
    m_pVelocityController = new VelocityController();
    m_bScratching = false;
    m_iScratchTime = 0;
    m_bEnableInertia = false;
    m_dRate = 0.;

    //m_pVelocityController->setPID(0.2, 1.0, 5.0);
    //m_pVelocityController->setPID(0.1, 0.0, 5.0);
    m_pVelocityController->setPID(0.3, 0.0, 0.00);
}

PositionScratchController::~PositionScratchController() {
    delete m_pScratchEnable;
    delete m_pScratchPosition;
    delete m_pVelocityController;
}

void PositionScratchController::process(double currentSample, bool paused, int iBufferSize) {
    bool scratchEnable = m_pScratchEnable->get() != 0;
    double scratchPosition = m_pScratchPosition->get();
    m_pVelocityController->setSamplesPerBuffer(iBufferSize);

    // The rate threshold above which disabling position scratching will enable
    // an 'inertia' mode.
    const double kThrowThreshold = 2.5;
    // The exponential decay factor that the rate will undergo if inertia mode
    // is triggered.
    const double kInertiaDecay = 0.9;
    // If we're playing, then do not decay rate below 1. If we're not playing,
    // then we want to decay all the way down to below 0.1
    const double kDecayThreshold = paused ? 0.1 : 1.0;

    if (m_bScratching) {
        if (scratchEnable || m_bEnableInertia) {
            // We were previously in scratch mode and are still in scratch mode
            // OR we are in inertia mode.

            if (scratchEnable) {
                // If we're scratching, clear the inertia flag. This case should
                // have been caught by the 'enable' case below, but just to make
                // sure.
                m_bEnableInertia = false;

                // Increment processing timer by one.
                m_iScratchTime += 1;

                // Set the scratch target to the current set position
                m_pVelocityController->setTarget(scratchPosition);

                m_dRate = m_pVelocityController->observation(currentSample, m_iScratchTime);
                //qDebug() << "continue" << m_dRate << iBufferSize;
            } else {
                // If we got here then we're not scratching and we're in inertia
                // mode. Take the previous rate that was set and apply an
                // exponential decay.
                m_dRate *= kInertiaDecay;

                // If the rate has decayed below the threshold, then leave
                // inertia mode.
                if (fabs(m_dRate) < kDecayThreshold) {
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
                m_iScratchTime = 0;
            }
            //qDebug() << "disable";
        }
    } else {
        if (scratchEnable) {
            // We were not previously in scratch mode but now are in scratch
            // mode. Enable scratching.
            m_bScratching = true;
            m_bEnableInertia = false;
            m_iScratchTime = 0;
            m_pVelocityController->reset(currentSample, m_iScratchTime, scratchPosition);
            m_dRate = m_pVelocityController->observation(currentSample, m_iScratchTime);
            //qDebug() << "enable" << m_dRate << currentSample;
        } else {
            // We were not previously in scratch mode are still not in scratch
            // mode. Do nothing
        }
    }
}

bool PositionScratchController::isEnabled() {
    return m_bScratching;
}

double PositionScratchController::getRate() {
    return m_dRate;
}
