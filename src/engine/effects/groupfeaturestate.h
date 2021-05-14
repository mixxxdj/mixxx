#pragma once

#include "proto/keys.pb.h"

struct GroupFeatureState {
    GroupFeatureState()
            : has_beat_length_sec(false),
              beat_length_sec(0.0),
              has_beat_fraction(false),
              beat_fraction(0.0),
              has_gain(false),
              gain(1.0) {
    }

    // The beat length in seconds.
    bool has_beat_length_sec;
    double beat_length_sec;

    // Fraction (0.0 to 1.0) of the current positions transition from the
    // previous beat to the next beat.
    bool has_beat_fraction;
    double beat_fraction;

    bool has_gain;
    double gain;
};
