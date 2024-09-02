#pragma once

#include "proto/keys.pb.h"

/// GroupFeatureState communicates metadata about EngineChannels to EffectProcessors.
struct GroupFeatureState {
    GroupFeatureState()
            : has_beat_length_frames(false),
              beat_length_frames(0.0),
              has_beat_fraction(false),
              beat_fraction_buffer_end(0.0),
              has_gain(false),
              gain(1.0) {
    }

    bool has_beat_length_frames;
    double beat_length_frames;

    // Beat fraction (0.0 to 1.0) of the position at the buffer end.
    // Previous beat is in the current or earlier buffer. The next beat
    // in the next or later buffer.
    bool has_beat_fraction;
    double beat_fraction_buffer_end;

    bool has_gain;
    double gain;
};
