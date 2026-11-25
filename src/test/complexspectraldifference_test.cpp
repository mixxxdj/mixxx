#include "analyzer/plugins/complexspectraldifference.h"

#include <gtest/gtest.h>

#include <cmath>
#include <complex>
#include <numbers>
#include <random>
#include <vector>

namespace {

constexpr int kWindowSize = 512;
constexpr int kStepSize = kWindowSize / 2;
constexpr int kFrames = 120;

/// qm-dsp's DetectionFunction with DF_COMPLEXSD, written out in double
/// precision with a naive DFT, atan2 and exp: Hann window, FFT shift by half
/// a window, then per bin |M1 - M e^(j princarg(phi - 2 phi1 + phi2))|.
class ReferenceComplexSpectralDifference {
  public:
    ReferenceComplexSpectralDifference()
            : m_magnitude(kWindowSize / 2 + 1),
              m_phase(kWindowSize / 2 + 1),
              m_olderPhase(kWindowSize / 2 + 1) {
    }

    double process(const double* pWindow) {
        std::vector<double> shifted(kWindowSize);
        for (int i = 0; i < kWindowSize; ++i) {
            const double hann = 0.5 - 0.5 * std::cos(2 * std::numbers::pi * i / kWindowSize);
            shifted[(i + kWindowSize / 2) % kWindowSize] = pWindow[i] * hann;
        }
        double sum = 0;
        for (int bin = 0; bin <= kWindowSize / 2; ++bin) {
            std::complex<double> value;
            for (int i = 0; i < kWindowSize; ++i) {
                value += shifted[i] *
                        std::polar(1.0, -2 * std::numbers::pi * bin * i / kWindowSize);
            }
            const double magnitude = std::abs(value);
            const double phase = std::atan2(value.imag(), value.real());
            const double deviation = std::remainder(
                    phase - 2 * m_phase[bin] + m_olderPhase[bin], 2 * std::numbers::pi);
            sum += std::abs(m_magnitude[bin] - std::polar(magnitude, deviation));
            m_olderPhase[bin] = m_phase[bin];
            m_phase[bin] = phase;
            m_magnitude[bin] = magnitude;
        }
        return sum;
    }

  private:
    std::vector<double> m_magnitude;
    std::vector<double> m_phase;
    std::vector<double> m_olderPhase;
};

/// Decaying tones, noise bursts and a stretch of digital silence.
std::vector<double> testSignal() {
    std::mt19937 generator(1);
    std::normal_distribution<double> noise(0, 0.05);
    std::vector<double> signal(kFrames * kStepSize + kWindowSize);
    for (std::size_t i = 0; i < signal.size(); ++i) {
        const double t = i / 44100.0;
        signal[i] = 0.3 * std::sin(2 * std::numbers::pi * 110 * t) *
                        std::exp(-8 * std::fmod(t, 0.1)) +
                0.2 * std::sin(2 * std::numbers::pi * 1234.5 * t) +
                noise(generator) * std::exp(-30 * std::fmod(t, 0.05));
    }
    std::fill(signal.begin() + signal.size() / 3,
            signal.begin() + signal.size() / 3 + 10 * kStepSize,
            0.0);
    return signal;
}

TEST(ComplexSpectralDifferenceTest, MatchesQmDsp) {
    const std::vector<double> signal = testSignal();
    mixxx::ComplexSpectralDifference detectionFunction(kWindowSize);
    ReferenceComplexSpectralDifference reference;
    double sum = 0;
    double differenceSum = 0;
    for (int frame = 0; frame < kFrames; ++frame) {
        const double* pWindow = signal.data() + frame * kStepSize;
        const double expected = reference.process(pWindow);
        const double actual = detectionFunction.process(pWindow);
        // Single precision FFT
        EXPECT_NEAR(actual, expected, 1e-3 * expected + 1e-6) << "frame " << frame;
        sum += expected;
        differenceSum += std::abs(actual - expected);
    }
    EXPECT_GT(sum, 0);
    EXPECT_LT(differenceSum, 1e-5 * sum);
}

TEST(ComplexSpectralDifferenceTest, SilenceIsZero) {
    const std::vector<double> silence(kWindowSize, 0.0);
    mixxx::ComplexSpectralDifference detectionFunction(kWindowSize);
    for (int frame = 0; frame < 3; ++frame) {
        EXPECT_EQ(detectionFunction.process(silence.data()), 0.0);
    }
}

} // namespace
