#pragma once

#include <cmath>

class SimpleBandPass {
  public:
    SimpleBandPass()
            : m_sampleRate(44100.0),
              m_z1(0.0),
              m_z2(0.0) {}

    void init(double sampleRate) {
        m_sampleRate = sampleRate;
        setParameters(1000.0, 0.707);  // 1kHz center, moderate Q
        reset();
    }

    void reset() {
        m_z1 = 0.0;
        m_z2 = 0.0;
    }

    void setParameters(double freq, double Q) {
        const double omega = 2.0 * M_PI * freq / m_sampleRate;
        const double alpha = sin(omega) / (2.0 * Q);

        const double b0 = alpha;
        const double b1 = 0.0;
        const double b2 = -alpha;
        const double a0 = 1.0 + alpha;
        const double a1 = -2.0 * cos(omega);
        const double a2 = 1.0 - alpha;

        m_b0 = b0 / a0;
        m_b1 = b1 / a0;
        m_b2 = b2 / a0;
        m_a1 = a1 / a0;
        m_a2 = a2 / a0;
    }

    inline float process(float input) {
        double output = m_b0 * input + m_z1;
        m_z1 = m_b1 * input - m_a1 * output + m_z2;
        m_z2 = m_b2 * input - m_a2 * output;
        return static_cast<float>(output);
    }

  private:
    double m_sampleRate;

    double m_b0, m_b1, m_b2;
    double m_a1, m_a2;

    double m_z1, m_z2;
};