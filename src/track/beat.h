#pragma once

#include <QDebug>
#include <QSharedPointer>

#include "proto/beats.pb.h"
#include "track/frame.h"
#include "track/timesignature.h"

namespace mixxx {
class Beat {
  public:
    enum Type {
        BEAT,
        DOWNBEAT
    };
    explicit Beat(FramePos framePos,
            Type type = BEAT,
            const TimeSignature& timeSignature = TimeSignature(),
            int beatIndex = 0,
            int barIndex = 0,
            int barRelativeBeatIndex = 0,
            bool hasMarker = false) {
        m_framePos = framePos;
        m_eType = type;
        m_iBeatIndex = beatIndex;
        m_iBarIndex = barIndex;
        m_iBarRelativeBeatIndex = barRelativeBeatIndex;
        m_timeSignature = timeSignature;
        m_bHasMarker = hasMarker;
    }
    int getBeatIndex() const {
        return m_iBeatIndex;
    }

    int getBarIndex() const {
        return m_iBarIndex;
    }

    int getBarRelativeBeatIndex() const {
        return m_iBarRelativeBeatIndex;
    }

    TimeSignature getTimeSignature() const {
        return m_timeSignature;
    }

    FramePos getFramePosition() const {
        return m_framePos;
    }

    Type getType() const {
        return m_eType;
    }

    bool hasMarker() const {
        return m_bHasMarker;
    }

  private:
    int m_iBeatIndex;
    int m_iBarIndex;
    int m_iBarRelativeBeatIndex;
    bool m_bHasMarker;
    Type m_eType;
    FramePos m_framePos;
    TimeSignature m_timeSignature;
};

using BeatList = QList<Beat>;

inline bool operator<(Beat beat1, Beat beat2) {
    return beat1.getFramePosition() < beat2.getFramePosition();
}

inline bool operator>(Beat beat1, Beat beat2) {
    return beat1.getFramePosition() > beat2.getFramePosition();
}

inline bool operator==(Beat beat1, Beat beat2) {
    return beat1.getFramePosition() == beat2.getFramePosition() &&
            beat1.getBeatIndex() == beat2.getBeatIndex() &&
            beat1.hasMarker() == beat2.hasMarker() &&
            beat1.getBarIndex() == beat2.getBarIndex() &&
            beat1.getBarRelativeBeatIndex() ==
            beat2.getBarRelativeBeatIndex() &&
            beat1.getType() == beat2.getType() &&
            beat1.getTimeSignature() == beat2.getTimeSignature();
}

inline bool operator!=(Beat beat1, Beat beat2) {
    return !(beat1 == beat2);
}

inline QDebug operator<<(QDebug dbg, Beat beat) {
    dbg << "[ Position:" << beat.getFramePosition()
        << " | Signature:" << beat.getTimeSignature()
        << " | Type:" << beat.getType() << " | BarIndex:" << beat.getBarIndex()
        << " | BeatIndex:" << beat.getBeatIndex() << "]"
        << " | BarRelativeBeatIndex:" << beat.getBarRelativeBeatIndex() << "]";
    return dbg;
}

const Beat kInvalidBeat = Beat(kInvalidFramePos);
}; // namespace mixxx