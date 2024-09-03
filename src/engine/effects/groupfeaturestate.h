#pragma once

#include <optional>

/// GroupFeatureState communicates metadata about EngineChannels to EffectProcessors.
struct GroupFeatureState {
    GroupFeatureState() {
    }

    std::optional<double> beat_length_frames;

    // Beat fraction (0.0 to 1.0) of the position at the buffer end.
    // Previous beat is in the current or earlier buffer. The next beat
    // in the next or later buffer.
    std::optional<double> beat_fraction_buffer_end;

    std::optional<double> gain;
};
