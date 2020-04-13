#include "audio/types.h"

namespace mixxx {

namespace audio {

QDebug operator<<(QDebug dbg, ChannelLayout arg) {
    switch (arg) {
    case ChannelLayout::Mono:
        return dbg << "Mono";
    case ChannelLayout::DualMono:
        return dbg << "DualMono";
    case ChannelLayout::Stereo:
        return dbg << "Stereo";
    }
    DEBUG_ASSERT(!"unreachable code");
    return dbg;
}

QDebug operator<<(QDebug dbg, SampleLayout arg) {
    switch (arg) {
    case SampleLayout::Planar:
        return dbg << "Planar";
    case SampleLayout::Interleaved:
        return dbg << "Interleaved";
    }
    DEBUG_ASSERT(!"unreachable code");
    return dbg;
}

QDebug operator<<(QDebug dbg, SampleRate arg) {
    return dbg
            << static_cast<SampleRate::value_t>(arg)
            << SampleRate::unit();
}

QDebug operator<<(QDebug dbg, Bitrate arg) {
    return dbg
            << static_cast<Bitrate::value_t>(arg)
            << Bitrate::unit();
}

} // namespace audio

} // namespace mixxx
