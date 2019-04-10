#ifndef MIXXX_CUE_H
#define MIXXX_CUE_H

#include <QObject>
#include <QMutex>
#include <QColor>

#include "track/trackid.h"
#include "util/color/predefinedcolor.h"
#include "util/memory.h"

class CueDAO;
class Track;

class Cue : public QObject {
  Q_OBJECT
  public:
    enum CueType {
        INVALID = 0,
        CUE     = 1, // hot cue
        LOAD    = 2, // the cue
        BEAT    = 3,
        LOOP    = 4,
        JUMP    = 5,
    };

    ~Cue() override = default;

    bool isDirty() const;
    int getId() const;
    TrackId getTrackId() const;

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

  signals:
    void updated();

  private:
    explicit Cue(TrackId trackId);
    Cue(int id, TrackId trackId, CueType type, double position, double length,
        int hotCue, QString label, PredefinedColorPointer color);
    void setDirty(bool dirty);
    void setId(int id);
    void setTrackId(TrackId trackId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    int m_iId;
    TrackId m_trackId;
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

#endif // MIXXX_CUE_H
