#include "effects/backends/builtin/pianosample.h"

#include <cmath>
#include <cstddef>
#include <vector>

namespace {

// A4 (440 Hz) matches the A4 reference used in kKeySemitoneOffset in the
// effect. Using A4 puts the 12-note range at C4–B4 (261–494 Hz), which
// sits in the mid-register and is easy to compare against both bass and
// melodic elements in a mix.
constexpr double kBaseFrequencyHz = 440.0;
constexpr double kDurationSeconds = 1.5;

// Leaving a small headroom below 1.0 prevents inter-sample clipping after
// pitch-shifting introduces brief interpolation overshoots.
constexpr double kNormalisationPeak = 0.88;

struct Harmonic {
    double relativeAmplitude;
    // Higher values decay faster, giving upper partials a shorter sustain and
    // producing the characteristic bright-then-mellow piano envelope.
    double decayRate;
};

// Amplitudes and decay rates approximate a real piano's spectral envelope.
constexpr Harmonic kHarmonics[] = {
        {1.00, 3.0},
        {0.60, 4.5},
        {0.40, 6.0},
        {0.22, 8.0},
        {0.14, 10.0},
        {0.08, 12.0},
        {0.05, 15.0},
        {0.03, 18.0},
        {0.02, 22.0},
};

} // namespace

std::vector<CSAMPLE> generatePianoSample(mixxx::audio::SampleRate sampleRate) {
    const int fs = sampleRate.value();
    const std::size_t nSamples =
            static_cast<std::size_t>(fs * kDurationSeconds);

    // Accumulate in double precision to avoid rounding error across the many
    // additive passes before we convert down to CSAMPLE at the end.
    std::vector<double> wave(nSamples, 0.0);

    for (std::size_t h = 0; h < std::size(kHarmonics); ++h) {
        const double freq = kBaseFrequencyHz * static_cast<double>(h + 1);
        for (std::size_t i = 0; i < nSamples; ++i) {
            const double t = static_cast<double>(i) / fs;
            wave[i] += kHarmonics[h].relativeAmplitude *
                    std::sin(2.0 * M_PI * freq * t) *
                    std::exp(-kHarmonics[h].decayRate * t);
        }
    }

    double peak = 0.0;
    for (double v : wave) {
        peak = std::max(peak, std::abs(v));
    }
    const double scale = (peak > 1e-9) ? kNormalisationPeak / peak : 1.0;

    std::vector<CSAMPLE> result(nSamples);
    for (std::size_t i = 0; i < nSamples; ++i) {
        result[i] = static_cast<CSAMPLE>(wave[i] * scale);
    }
    return result;
}
