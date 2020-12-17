#pragma once

#include <QColor>
#include <QMutex>
#include <QObject>
#include <memory>
#include <type_traits> // static_assert

#include "audio/types.h"
#include "track/cueinfo.h"
#include "util/color/rgbcolor.h"
#include "util/db/dbid.h"

class CueDAO;
class Track;

class Cue : public QObject {
    Q_OBJECT

  public:
    /// A position value for the cue that signals its position is not set
    static constexpr double kNoPosition = -1.0;

    /// Invalid hot cue index
    static constexpr int kNoHotCue = -1;

    static_assert(kNoHotCue != mixxx::kFirstHotCueIndex,
            "Conflicting definitions of invalid and first hot cue index");

    Cue();
    Cue(
            const mixxx::CueInfo& cueInfo,
            mixxx::audio::SampleRate sampleRate,
            bool setDirty);
    /// Load entity from database.
    Cue(
            DbId id,
            mixxx::CueType type,
            double position,
            double length,
            int hotCue,
            const QString& label,
            mixxx::RgbColor color);
    ~Cue() override = default;

    bool isDirty() const;
    DbId getId() const;

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
    void setLabel(const QString& label);

    mixxx::RgbColor getColor() const;
    void setColor(mixxx::RgbColor color);

    double getEndPosition() const;

    mixxx::CueInfo getCueInfo(
            mixxx::audio::SampleRate sampleRate) const;

  signals:
    void updated();

  private:
    void setDirty(bool dirty);

    void setId(DbId dbId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    DbId m_dbId;
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
