#include "sources/soundsourceprovider.h"

namespace mixxx {

QDebug operator<<(QDebug dbg, SoundSourceProviderPriority arg) {
    switch (arg) {
    case SoundSourceProviderPriority::Lowest:
        return dbg << static_cast<int>(arg) << "(lowest)";
    case SoundSourceProviderPriority::Lower:
        return dbg << static_cast<int>(arg) << "(lower)";
    case SoundSourceProviderPriority::Default:
        return dbg << static_cast<int>(arg) << "(default)";
    case SoundSourceProviderPriority::Higher:
        return dbg << static_cast<int>(arg) << "(higher)";
    case SoundSourceProviderPriority::Highest:
        return dbg << static_cast<int>(arg) << "(highest)";
    default:
        DEBUG_ASSERT(!"unexpected SoundSourceProviderPriority");
        return dbg;
    }
}

} // namespace mixxx
