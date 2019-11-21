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
    enum class Type {
        Invalid = 0,
        HotCue  = 1,
        MainCue = 2,
        Beat    = 3, // unused (what is this for?)
        Loop    = 4,
        Jump    = 5,
        Intro   = 6,
        Outro   = 7,
        AudibleSound = 8, // range that covers beginning and end of audible sound;
                          // not shown to user
    };

    static constexpr double kNoPosition = -1.0;
    static const int kNoHotCue = -1;

    ~Cue() override = default;

    bool isDirty() const;
    int getId() const;
    TrackId getTrackId() const;

    Cue::Type getType() const;
    void setType(Cue::Type type);

    double getPosition() const;
    void setStartPosition(double samplePosition);
    void setEndPosition(double samplePosition);

    double getLength() const;

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
    Cue(int id, TrackId trackId, Cue::Type type, double position, double length,
        int hotCue, QString label, PredefinedColorPointer color);
    void setDirty(bool dirty);
    void setId(int id);
    void setTrackId(TrackId trackId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    int m_iId;
    TrackId m_trackId;
    Cue::Type m_type;
    double m_sampleStartPosition;
    double m_sampleEndPosition;
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
        : m_position(0.0) {}
    CuePosition(double position)
        : m_position(position) {}

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

inline
bool operator!=(const CuePosition& lhs, const CuePosition& rhs) {
    return !(lhs == rhs);
}

inline
QDebug operator<<(QDebug dbg, const CuePosition& arg) {
    return dbg << "position =" << arg.getPosition();
}

#endif // MIXXX_CUE_H
