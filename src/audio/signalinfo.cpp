#include "audio/signalinfo.h"

namespace mixxx {

namespace audio {

bool operator==(
        const SignalInfo& lhs,
        const SignalInfo& rhs) {
    return lhs.getChannelCount() == rhs.getChannelCount() &&
            lhs.getSampleLayout() == rhs.getSampleLayout() &&
            lhs.getSampleRate() == rhs.getSampleRate();
}

QDebug
operator<<(QDebug dbg, const SignalInfo& arg) {
    dbg << "SignalInfo{";
    arg.dbgChannelCount(dbg);
    arg.dbgSampleLayout(dbg);
    arg.dbgSampleRate(dbg);
    dbg << '}';
    return dbg;
}

} // namespace audio

} // namespace mixxx
