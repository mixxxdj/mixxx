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
    enum class Source {
        Unknown   = 0,
        Automatic = 1,
        Manual    = 2,
    };

    enum class Type {
        Invalid = 0,
        Hotcue  = 1,
        MainCue = 2,
        Beat    = 3, // unused (what is this for?)
        Loop    = 4,
        Jump    = 5,
        Intro   = 6,
        Outro   = 7,
    };

    ~Cue() override = default;

    bool isDirty() const;
    int getId() const;
    TrackId getTrackId() const;

    Cue::Source getSource() const;
    void setSource(Cue::Source source);

    Cue::Type getType() const;
    void setType(Cue::Type type);

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
    Cue(int id, TrackId trackId, Cue::Source source, Cue::Type type, double position, double length,
        int hotCue, QString label, PredefinedColorPointer color);
    void setDirty(bool dirty);
    void setId(int id);
    void setTrackId(TrackId trackId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    int m_iId;
    TrackId m_trackId;
    Cue::Source m_source;
    Cue::Type m_type;
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
        : m_position(0.0), m_source(Cue::Source::Unknown) {}
    CuePosition(double position, Cue::Source source)
        : m_position(position), m_source(source) {}

    double getPosition() const {
        return m_position;
    }

    void setPosition(double position) {
        m_position = position;
    }

    Cue::Source getSource() const {
        return m_source;
    }

    void setSource(Cue::Source source) {
        m_source = source;
    }

    void set(double position, Cue::Source source) {
        m_position = position;
        m_source = source;
    }

    void reset() {
        m_position = 0.0;
        m_source = Cue::Source::Unknown;
    }

  private:
    double m_position;
    Cue::Source m_source;
};

bool operator==(const CuePosition& lhs, const CuePosition& rhs);

inline
bool operator!=(const CuePosition& lhs, const CuePosition& rhs) {
    return !(lhs == rhs);
}

inline
QDebug operator<<(QDebug dbg, const CuePosition& arg) {
    return dbg << "position =" << arg.getPosition() << "/" << "source =" << static_cast<int>(arg.getSource());
}

#endif // MIXXX_CUE_H
