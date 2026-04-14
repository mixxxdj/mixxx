#pragma once

#include <QFlags>

#include "audio/types.h"

// Audio signal-path channel-count constants and stem-channel types.
//
// These describe the fixed channel configuration used throughout the engine
// and analysis pipeline. They live in audio/ because they are properties of
// the audio signal path, not of the engine subsystem.

namespace mixxx {

static constexpr audio::ChannelCount kEngineChannelOutputCount =
        audio::ChannelCount::stereo();
static constexpr audio::ChannelCount kMaxEngineChannelInputCount =
        audio::ChannelCount::stem();
// Always defined (not gated on __STEM__) because the waveform data struct
// must stay consistent regardless of whether STEM support is compiled in.
constexpr int kMaxSupportedStems = 4;

#ifdef __STEM__
enum class StemChannel {
    First = 0x1,
    Second = 0x2,
    Third = 0x4,
    Fourth = 0x8,

    None = 0,
    All = First | Second | Third | Fourth
};
Q_DECLARE_FLAGS(StemChannelSelection, StemChannel);
#endif

} // namespace mixxx
