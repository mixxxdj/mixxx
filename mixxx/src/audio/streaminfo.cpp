#include "audio/streaminfo.h"

namespace mixxx {

namespace audio {

bool operator==(
        const StreamInfo& lhs,
        const StreamInfo& rhs) {
    return lhs.getSignalInfo() == rhs.getSignalInfo() &&
            lhs.getBitrate() == rhs.getBitrate() &&
            lhs.getDuration() == rhs.getDuration();
}

QDebug
operator<<(QDebug dbg, const StreamInfo& arg) {
    dbg << "StreamInfo{";
    arg.dbgSignalInfo(dbg);
    arg.dbgBitrate(dbg);
    arg.dbgDuration(dbg);
    dbg << '}';
    return dbg;
}

} // namespace audio

} // namespace mixxx
