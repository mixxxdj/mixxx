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
            << QString::number(arg).toLocal8Bit().constData()
            << SampleRate::unit();
}

QDebug operator<<(QDebug dbg, Bitrate arg) {
    return dbg
            << QString::number(arg).toLocal8Bit().constData()
            << Bitrate::unit();
}

} // namespace audio

} // namespace mixxx
