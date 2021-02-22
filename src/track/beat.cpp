#include "beat.h"

namespace mixxx {
Beat::Beat(FramePos framePos,
        BeatType type,
        const TimeSignature& timeSignature,
        const Bpm& bpm,
        int beatIndex,
        int barIndex,
        uint beatInBarIndex,
        BeatMarkers markers)
        : m_framePos(framePos),
          m_eType(type),
          m_iBeatIndex(beatIndex),
          m_iBarIndex(barIndex),
          m_iBeatInBarIndex(beatInBarIndex),
          m_timeSignature(timeSignature),
          m_bpm(bpm),
          m_eMarkers(markers) {
}

bool operator==(const Beat& beat1, const Beat& beat2) {
    return beat1.framePosition() == beat2.framePosition() &&
            beat1.beatIndex() == beat2.beatIndex() &&
            beat1.markers() == beat2.markers() &&
            beat1.barIndex() == beat2.barIndex() &&
            beat1.beatInBarIndex() ==
            beat2.beatInBarIndex() &&
            beat1.type() == beat2.type() &&
            beat1.timeSignature() == beat2.timeSignature() &&
            beat1.bpm() == beat2.bpm();
}

QDebug operator<<(QDebug dbg, const Beat& beat) {
    dbg << "[ Position:" << beat.framePosition()
        << " | Signature:" << beat.timeSignature()
        << " | Type:"
        << (beat.type() == BeatType::Beat
                           ? "Beat"
                           : (beat.type() == BeatType::Downbeat) ? "Downbeat"
                                                                 : "None")
        << " | BarIndex:" << beat.barIndex()
        << " | BeatIndex:" << beat.beatIndex()
        << " | BeatInBarIndex:" << beat.beatInBarIndex()
        << " | BPM:" << beat.bpm() << " | Markers:" << beat.markers() << "]";
    return dbg;
}
} // namespace mixxx
