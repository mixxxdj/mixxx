#include "audio/frame.h"

namespace mixxx {
namespace audio {

QDebug operator<<(QDebug dbg, FramePos arg) {
    if (arg.isValid()) {
        QDebugStateSaver saver(dbg);
        // 9 allows precise display of up to 6 h @ 44100 Hz
        dbg.nospace() << "FramePos(" << qSetRealNumberPrecision(9) << arg.value() << ")";
    } else {
        dbg << "FramePos()";
    }
    return dbg;
}

} // namespace audio
} // namespace mixxx
