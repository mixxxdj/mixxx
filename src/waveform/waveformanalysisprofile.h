#pragma once

namespace mixxx {

enum class WaveformAnalysisProfile {
    Legacy,
    Perceptual3Band,
};

inline constexpr float kPerceptual3BandMaximumValue = 127.0f;

} // namespace mixxx
