#ifndef MIXXX_CUE_H
#define MIXXX_CUE_H

#include <QObject>
#include <QMutex>
#include <QColor>

#include "track/trackid.h"
#include "util/color/predefinedcolor.h"
#include "util/memory.h"

class CuePosition;
class CueDAO;
class Track;

class Cue : public QObject {
    Q_OBJECT

  public:
    enum CueSource {
        UNKNOWN   = 0,
        AUTOMATIC = 1,
        MANUAL    = 2,
    };

    enum CueType {
        INVALID = 0,
        CUE     = 1, // hot cue
        LOAD    = 2, // the cue
        BEAT    = 3,
        LOOP    = 4,
        JUMP    = 5,
        INTRO   = 6,
        OUTRO   = 7,
    };

    ~Cue() override = default;

    bool isDirty() const;
    int getId() const;
    TrackId getTrackId() const;

    CueSource getSource() const;
    void setSource(CueSource source);

    CueType getType() const;
    void setType(CueType type);

    double getPosition() const;
    void setPosition(double samplePosition);

    double getLength() const;
    void setLength(double length);

    int getHotCue() const;
    void setHotCue(int hotCue);

    QString getLabel() const;
    void setLabel(QString label);

    PredefinedColorPointer getColor() const;
    void setColor(PredefinedColorPointer color);

    double getEndPosition() const;

  signals:
    void updated();

  private:
    explicit Cue(TrackId trackId);
    Cue(int id, TrackId trackId, CueSource source, CueType type, double position, double length,
        int hotCue, QString label, PredefinedColorPointer color);
    void setDirty(bool dirty);
    void setId(int id);
    void setTrackId(TrackId trackId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    int m_iId;
    TrackId m_trackId;
    CueSource m_source;
    CueType m_type;
    double m_samplePosition;
    double m_length;
    int m_iHotCue;
    QString m_label;
    PredefinedColorPointer m_color;

    friend class Track;
    friend class CueDAO;
};

class CuePointer: public std::shared_ptr<Cue> {
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
        : m_position(0.0), m_source(Cue::UNKNOWN) {}
    CuePosition(double position, Cue::CueSource source)
        : m_position(position), m_source(source) {}

    double getPosition() const {
        return m_position;
    }

    void setPosition(double position) {
        m_position = position;
    }

    Cue::CueSource getSource() const {
        return m_source;
    }

    void setSource(Cue::CueSource source) {
        m_source = source;
    }

    void set(double position, Cue::CueSource source) {
        m_position = position;
        m_source = source;
    }

    void reset() {
        m_position = 0.0;
        m_source = Cue::UNKNOWN;
    }

  private:
    double m_position;
    Cue::CueSource m_source;
};

bool operator==(const CuePosition& lhs, const CuePosition& rhs);

inline
bool operator!=(const CuePosition& lhs, const CuePosition& rhs) {
    return !(lhs == rhs);
}

inline
QDebug operator<<(QDebug dbg, const CuePosition& arg) {
    return dbg << "position =" << arg.getPosition() << "/" << "source =" << arg.getSource();
}

#endif // MIXXX_CUE_H
