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
    Beat(FramePos framePos,
            Type type = BEAT,
            TimeSignature timeSignature = TimeSignature(),
            int beatIndex = 0,
            int barIndex = 0) {
        m_framePos = framePos;
        m_eType = type;
        m_iBeatIndex = beatIndex;
        m_iBarIndex = barIndex;
        m_timeSignature = timeSignature;
    }
    int getBeatIndex() const {
        return m_iBeatIndex;
    }

    int getBarIndex() const {
        return m_iBarIndex;
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

  private:
    int m_iBeatIndex;
    int m_iBarIndex;
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

inline QDebug operator<<(QDebug dbg, Beat beat) {
    dbg << "[ Position:" << beat.getFramePosition()
        << " | Signature:" << beat.getTimeSignature()
        << " | Type:" << beat.getType() << " | BarIndex:" << beat.getBarIndex()
        << " | BeatIndex:" << beat.getBeatIndex() << "]";
    return dbg;
}

//
//bool operator<(const Beat& beat1, const Beat& beat2) {
//}
}; // namespace mixxx