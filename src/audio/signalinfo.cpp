#include "audio/signalinfo.h"

namespace mixxx {

namespace audio {

bool operator==(
        const SignalInfo& lhs,
        const SignalInfo& rhs) {
    return lhs.getChannelCount() == rhs.getChannelCount() &&
            lhs.getSampleRate() == rhs.getSampleRate();
}

QDebug
operator<<(QDebug dbg, const SignalInfo& arg) {
    dbg << "SignalInfo{";
    arg.dbgChannelCount(dbg);
    arg.dbgSampleRate(dbg);
    dbg << '}';
    return dbg;
}

} // namespace audio

} // namespace mixxx
