#ifndef MIXXX_CUE_H
#define MIXXX_CUE_H

#include <QObject>
#include <QMutex>
#include <QSharedPointer>
#include <QColor>

#include "track/trackid.h"

class CueDAO;
class Track;

class Cue : public QObject {
  Q_OBJECT
  public:
    enum CueType {
        INVALID = 0,
        CUE,
        LOAD,
        BEAT,
        LOOP,
        JUMP,
    };

    virtual ~Cue();

    bool isDirty() const;
    int getId() const;
    TrackId getTrackId() const;

    CueType getType() const;
    void setType(CueType type);

    int getPosition() const;
    void setPosition(int position);

    int getLength() const;
    void setLength(int length);

    int getHotCue() const;
    void setHotCue(int hotCue);

    QString getLabel() const;
    void setLabel(QString label);

    QColor getColor() const;
    void setColor(QColor color);

  signals:
    void updated();

  private:
    explicit Cue(TrackId trackId);
    Cue(int id, TrackId trackId, CueType type, int position, int length,
        int hotCue, QString label, QColor color);
    void setDirty(bool dirty);
    void setId(int id);
    void setTrackId(TrackId trackId);

    mutable QMutex m_mutex;

    bool m_bDirty;
    int m_iId;
    TrackId m_trackId;
    CueType m_type;
    int m_iPosition;
    int m_iLength;
    int m_iHotCue;
    QString m_label;
    QColor m_color;

    friend class Track;
    friend class CueDAO;
};

class CuePointer: public QSharedPointer<Cue> {
  public:
    CuePointer() {}
    explicit CuePointer(Cue* pCue)
          : QSharedPointer<Cue>(pCue, deleteLater) {
    }

    // TODO(uklotzde): Remove these functions after migration
    // from QSharedPointer to std::shared_ptr
    Cue* get() const {
        return data();
    }
    void reset() {
        clear();
    }

  private:
    static void deleteLater(Cue* pCue) {
        if (pCue != nullptr) {
            pCue->deleteLater();
        }
    }
};

#endif // MIXXX_CUE_H
