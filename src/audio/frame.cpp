#include "audio/frame.h"

namespace mixxx {
namespace audio {

QDebug operator<<(QDebug dbg, FramePos arg) {
    if (arg.isValid()) {
        QDebugStateSaver saver(dbg);
        dbg.nospace() << "FramePos(" << arg.value() << ")";
    } else {
        dbg << "FramePos()";
    }
    return dbg;
}

} // namespace audio
} // namespace mixxx
