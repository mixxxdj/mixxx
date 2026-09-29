#pragma once

#include <algorithm>
#include <array>

#include "waveform/waveform.h"
#include "waveform/waveformanalysisprofile.h"

namespace mixxx::analyzer {

class PerceptualWaveformLut {
  public:
    static float applyNormalized(float value, int band) {
        value = std::clamp(value, 0.0f, 1.0f);
        if (value == 0.0f) {
            return 0.0f;
        }
        return interpolate(value * 255.0f, band) / 255.0f;
    }

  private:
    struct Knot {
        float input;
        float output;
    };

    static constexpr float kLowTailSlope = 0.136f;
    static constexpr float kMaximumOutput = mixxx::kPerceptual3BandMaximumValue;

    static const std::array<Knot, 10>& lowKnots() {
        static constexpr std::array knots{
                Knot{0.0f, 0.0f},
                Knot{8.0f, 8.5f},
                Knot{16.0f, 16.0f},
                Knot{32.0f, 27.0f},
                Knot{48.0f, 37.0f},
                Knot{64.0f, 44.0f},
                Knot{80.0f, 53.0f},
                Knot{96.0f, 58.0f},
                Knot{112.0f, 67.5f},
                Knot{128.0f, 77.0f}};
        return knots;
    }

    static const std::array<Knot, 11>& midKnots() {
        static constexpr std::array knots{
                Knot{0.0f, 0.0f},
                Knot{8.0f, 4.0f},
                Knot{16.0f, 15.0f},
                Knot{32.0f, 26.0f},
                Knot{48.0f, 35.0f},
                Knot{64.0f, 46.0f},
                Knot{80.0f, 57.0f},
                Knot{96.0f, 66.0f},
                Knot{112.0f, 74.0f},
                Knot{128.0f, 81.0f},
                Knot{160.0f, 92.0f}};
        return knots;
    }

    static const std::array<Knot, 9>& highKnots() {
        static constexpr std::array knots{
                Knot{0.0f, 0.0f},
                Knot{8.0f, 4.0f},
                Knot{16.0f, 13.0f},
                Knot{32.0f, 29.0f},
                Knot{48.0f, 58.0f},
                Knot{64.0f, 77.0f},
                Knot{80.0f, 90.0f},
                Knot{96.0f, 97.0f},
                Knot{112.0f, 100.0f}};
        return knots;
    }

    template<std::size_t Size>
    static float interpolate(float value, const std::array<Knot, Size>& knots) {
        if (value <= knots.front().input) {
            return knots.front().output;
        }
        for (std::size_t index = 1; index < knots.size(); ++index) {
            if (value <= knots[index].input) {
                const Knot& lower = knots[index - 1];
                const Knot& upper = knots[index];
                const float factor = (value - lower.input) / (upper.input - lower.input);
                return lower.output + factor * (upper.output - lower.output);
            }
        }
        return knots.back().output;
    }

    static float interpolateLow(float value) {
        const auto& knots = lowKnots();
        if (value <= knots.back().input) {
            return interpolate(value, knots);
        }
        return std::min(
                kMaximumOutput,
                knots.back().output + kLowTailSlope * (value - knots.back().input));
    }

    static float interpolate(float value, int band) {
        switch (band) {
        case Low:
            return interpolateLow(value);
        case Mid:
            return interpolate(value, midKnots());
        case High:
            return interpolate(value, highKnots());
        default:
            return 0.0f;
        }
    }
};

} // namespace mixxx::analyzer
