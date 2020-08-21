#pragma once

#include <QDebug>
#include <QSharedPointer>

#include "proto/beats.pb.h"
#include "track/bpm.h"
#include "track/frame.h"
#include "track/timesignature.h"

namespace mixxx {
class Beat {
  public:
    enum Type {
        BEAT,
        DOWNBEAT
    };
    enum Marker {
        NONE = 0,
        TIME_SIGNATURE = 1 << 0,
        BPM = 1 << 1
    };
    Q_DECLARE_FLAGS(Markers, Marker)
    explicit Beat(FramePos framePos,
            Type type = BEAT,
            const TimeSignature& timeSignature = TimeSignature(),
            const Bpm& bpm = Bpm(),
            int beatIndex = 0,
            int barIndex = 0,
            int barRelativeBeatIndex = 0,
            Markers markers = Marker::NONE)
            : m_framePos(framePos),
              m_eType(type),
              m_iBeatIndex(beatIndex),
              m_iBarIndex(barIndex),
              m_iBarRelativeBeatIndex(barRelativeBeatIndex),
              m_timeSignature(timeSignature),
              m_bpm(bpm),
              m_eMarkers(markers) {
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

    Markers getMarkers() const {
        return m_eMarkers;
    }

    Bpm getBpm() const {
        return m_bpm;
    }

  private:
    FramePos m_framePos;
    Type m_eType;
    int m_iBeatIndex;
    int m_iBarIndex;
    int m_iBarRelativeBeatIndex;
    TimeSignature m_timeSignature;
    // This BPM value represents the instantaneous BPM at this beat.
    Bpm m_bpm;
    Markers m_eMarkers;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Beat::Markers);

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
            beat1.getMarkers() == beat2.getMarkers() &&
            beat1.getBarIndex() == beat2.getBarIndex() &&
            beat1.getBarRelativeBeatIndex() ==
            beat2.getBarRelativeBeatIndex() &&
            beat1.getType() == beat2.getType() &&
            beat1.getTimeSignature() == beat2.getTimeSignature() &&
            beat1.getBpm() == beat2.getBpm();
}

inline bool operator!=(Beat beat1, Beat beat2) {
    return !(beat1 == beat2);
}

inline QDebug operator<<(QDebug dbg, Beat beat) {
    dbg << "[ Position:" << beat.getFramePosition()
        << " | Signature:" << beat.getTimeSignature()
        << " | Type:" << beat.getType()
        << " | BarIndex:" << beat.getBarIndex()
        << " | BeatIndex:" << beat.getBeatIndex()
        << " | BarRelativeBeatIndex:" << beat.getBarRelativeBeatIndex()
        << " | BPM:" << beat.getBpm()
        << " | Markers:" << beat.getMarkers() << "]";
    return dbg;
}

const Beat kInvalidBeat = Beat(kInvalidFramePos);
}; // namespace mixxx