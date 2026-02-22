#pragma once

#include <optional>

struct GroupFeatureBeatLength {
    // Beat length adjusted by the rate slider
    double seconds;
    // Rate change by temporary actions like scratching
    // and not the rate slider.
    double scratch_rate;
};

/// GroupFeatureState communicates metadata about EngineChannels to EffectProcessors.
struct GroupFeatureState {
    GroupFeatureState() = default;

    std::optional<GroupFeatureBeatLength> beat_length;

    // Beat fraction (0.0 to 1.0) of the position at the buffer end.
    // Previous beat is in the current or earlier buffer. The next beat
    // in the next or later buffer.
    std::optional<double> beat_fraction_buffer_end;

    std::optional<double> gain;
};
