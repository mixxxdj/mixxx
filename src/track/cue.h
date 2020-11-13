#pragma once

#include <QColor>
#include <QMutex>
#include <QObject>

#include "audio/types.h"
#include "track/cueinfo.h"
#include "track/trackid.h"
#include "util/color/rgbcolor.h"
#include "util/memory.h"

class CuePosition;
class CueDAO;
class Track;

class Cue : public QObject {
    Q_OBJECT

  public:
    static constexpr double kNoPosition = -1.0;
    static constexpr int kNoHotCue = -1;

    Cue();
    Cue(
            const mixxx::CueInfo& cueInfo,
            mixxx::audio::SampleRate sampleRate,
            bool setDirty);
    Cue(
            int id,
            TrackId trackId,
            mixxx::CueType type,
            double position,
            double length,
            int hotCue,
            QString label,
            mixxx::RgbColor color);
    ~Cue() override = default;

    bool isDirty() const;
    int getId() const;
    TrackId getTrackId() const;

    mixxx::CueType getType() const;
    void setType(mixxx::CueType type);

    double getPosition() const;
    void setStartPosition(
            double samplePosition = kNoPosition);
    void setEndPosition(
            double samplePosition = kNoPosition);
    void shiftPositionFrames(double frameOffset);

    double getLength() const;

    int getHotCue() const;
    void setHotCue(
            int hotCue = kNoHotCue);

    QString getLabel() const;
    void setLabel(
            QString label = QString());

    mixxx::RgbColor getColor() const;
    void setColor(mixxx::RgbColor color);

    double getEndPosition() const;

    mixxx::CueInfo getCueInfo(
            mixxx::audio::SampleRate sampleRate) const;

  signals:
    void updated();

  private:
    void setDirty(bool dirty);

    void setId(int id);
    void setTrackId(TrackId trackId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    int m_iId;
    TrackId m_trackId;
    mixxx::CueType m_type;
    double m_sampleStartPosition;
    double m_sampleEndPosition;
    int m_iHotCue;
    QString m_label;
    mixxx::RgbColor m_color;

    friend class Track;
    friend class CueDAO;
};

class CuePointer : public std::shared_ptr<Cue> {
  public:
    CuePointer() = default;
    explicit CuePointer(Cue* pCue)
            : std::shared_ptr<Cue>(pCue, deleteLater) {
    }

  private:
    static void deleteLater(Cue* pCue);
};

class CuePosition {
  public:
    CuePosition()
            : m_position(0.0) {
    }
    CuePosition(double position)
            : m_position(position) {
    }

    double getPosition() const {
        return m_position;
    }

    void setPosition(double position) {
        m_position = position;
    }

    void set(double position) {
        m_position = position;
    }

    void reset() {
        m_position = 0.0;
    }

  private:
    double m_position;
};

bool operator==(const CuePosition& lhs, const CuePosition& rhs);

inline bool operator!=(const CuePosition& lhs, const CuePosition& rhs) {
    return !(lhs == rhs);
}

inline QDebug operator<<(QDebug dbg, const CuePosition& arg) {
    return dbg << "position =" << arg.getPosition();
}
