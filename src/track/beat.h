#pragma once

#include <QDebug>
#include <QSharedPointer>

#include "proto/beats.pb.h"
#include "track/bpm.h"
#include "track/frame.h"
#include "track/timesignature.h"

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
            BeatMarkers markers = BeatMarker::None);

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

inline bool operator<(const Beat& beat1, const Beat& beat2) {
    return beat1.framePosition() < beat2.framePosition();
}

inline bool operator<=(const Beat& beat1, const Beat& beat2) {
    return beat1.framePosition() <= beat2.framePosition();
}

inline bool operator>(const Beat& beat1, const Beat& beat2) {
    return beat1.framePosition() > beat2.framePosition();
}

inline bool operator>=(const Beat& beat1, const Beat& beat2) {
    return beat1.framePosition() >= beat2.framePosition();
}

bool operator==(const Beat& beat1, const Beat& beat2);

inline bool operator!=(const Beat& beat1, const Beat& beat2) {
    return !(beat1 == beat2);
}

QDebug operator<<(QDebug dbg, const Beat& beat);
constexpr int kFirstBeatIndex = 0;
}; // namespace mixxx
