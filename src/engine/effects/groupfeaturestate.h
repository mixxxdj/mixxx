#ifndef GROUPFEATURESTATE_H
#define GROUPFEATURESTATE_H

#include "proto/keys.pb.h"

struct GroupFeatureState {
    GroupFeatureState()
            : has_current_position(false),
              current_position(0.0),
              has_rms_volume_sum(false),
              rms_volume_sum(0.0),
              has_file_bpm(false),
              file_bpm(0.0),
              has_bpm(false),
              bpm(0.0),
              has_next_beat(false),
              next_beat(false),
              has_prev_beat(false),
              prev_beat(0.0),
              has_beat_length(false),
              beat_length(0.0),
              has_beat_fraction(false),
              beat_fraction(0.0),
              has_file_key(false),
              file_key(mixxx::track::io::key::INVALID),
              has_key(false),
              key(mixxx::track::io::key::INVALID),
              has_gain(false),
              gain(1.0) {
    }

    // The current player position (if it is a player with the concept of a
    // position) in stereo samples.
    bool has_current_position;
    double current_position;

    // The RMS volume of this group. This is taken from the VU meter for the
    // group which depends on the output of the effects system so this is always
    // delayed by one callback.
    bool has_rms_volume_sum;
    double rms_volume_sum;

    // The file/source BPM of this group (before rate-adjusting)
    bool has_file_bpm;
    double file_bpm;

    // The effective BPM of this group (rate-adjusted).
    bool has_bpm;
    double bpm;

    // The next beat location in stereo samples.
    bool has_next_beat;
    double next_beat;

    // The previous beat location in stereo samples.
    bool has_prev_beat;
    double prev_beat;

    // The beat length in stereo samples.
    bool has_beat_length;
    double beat_length;

    // Fraction (0.0 to 1.0) of the current positions transition from the
    // previous beat to the next beat.
    bool has_beat_fraction;
    double beat_fraction;

    // The file/source musical key of this group (before pitch-shifting).
    bool has_file_key;
    mixxx::track::io::key::ChromaticKey file_key;

    // The effective musical key of this group (pitch-shifted).
    bool has_key;
    mixxx::track::io::key::ChromaticKey key;

    bool has_gain;
    double gain;
};

#endif /* GROUPFEATURESTATE_H */
