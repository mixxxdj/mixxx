#pragma once

#include <QDebug>
#include <QSharedPointer>

#include "proto/beats.pb.h"
#include "track/bpm.h"
#include "track/frame.h"
#include "track/timesignature.h"

namespace {
constexpr int kNegativeInfinity = std::numeric_limits<int>::lowest();
}
namespace mixxx {
enum class BeatType {
    Beat,
    Downbeat
};

enum class BeatMarker {
    None = 0,
    TimeSignature = 1 << 0,
    Bpm = 1 << 1
};
Q_DECLARE_FLAGS(BeatMarkers, BeatMarker)

class Beat {
  public:
    explicit Beat(FramePos framePos,
            BeatType type = BeatType::Beat,
            const TimeSignature& timeSignature = TimeSignature(),
            const Bpm& bpm = Bpm(),
            int beatIndex = 0,
            int barIndex = 0,
            uint beatInBarIndex = 0,
            BeatMarkers markers = BeatMarker::None)
            : m_framePos(framePos),
              m_eType(type),
              m_iBeatIndex(beatIndex),
              m_iBarIndex(barIndex),
              m_iBeatInBarIndex(beatInBarIndex),
              m_timeSignature(timeSignature),
              m_bpm(bpm),
              m_eMarkers(markers) {
    }

    int beatIndex() const {
        return m_iBeatIndex;
    }

    int barIndex() const {
        return m_iBarIndex;
    }

    uint beatInBarIndex() const {
        return m_iBeatInBarIndex;
    }

    TimeSignature timeSignature() const {
        return m_timeSignature;
    }

    FramePos framePosition() const {
        return m_framePos;
    }

    BeatType type() const {
        return m_eType;
    }

    BeatMarkers markers() const {
        return m_eMarkers;
    }

    Bpm bpm() const {
        return m_bpm;
    }

  private:
    FramePos m_framePos;
    BeatType m_eType;
    int m_iBeatIndex;
    int m_iBarIndex;
    uint m_iBeatInBarIndex;
    TimeSignature m_timeSignature;
    // This BPM value represents the instantaneous BPM at this beat.
    Bpm m_bpm;
    BeatMarkers m_eMarkers;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BeatMarkers);

using BeatList = QList<Beat>;

inline bool operator<(Beat beat1, Beat beat2) {
    return beat1.framePosition() < beat2.framePosition();
}

inline bool operator<=(Beat beat1, Beat beat2) {
    return beat1.framePosition() <= beat2.framePosition();
}

inline bool operator>(Beat beat1, Beat beat2) {
    return beat1.framePosition() > beat2.framePosition();
}

inline bool operator>=(Beat beat1, Beat beat2) {
    return beat1.framePosition() >= beat2.framePosition();
}

inline bool operator==(Beat beat1, Beat beat2) {
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

inline bool operator!=(Beat beat1, Beat beat2) {
    return !(beat1 == beat2);
}

QDebug operator<<(QDebug dbg, Beat beat) {
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

const Beat kInvalidBeat = Beat(kInvalidFramePos,
        BeatType::Beat,
        TimeSignature(),
        Bpm(),
        kNegativeInfinity,
        kNegativeInfinity,
        kNegativeInfinity);

constexpr int kFirstBeatIndex = 0;
}; // namespace mixxx
