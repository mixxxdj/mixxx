// cue.h
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUE_H
#define CUE_H

#include <QObject>
#include <QMutex>
#include <QSharedPointer>

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
    void setLabel(const QString label);

  signals:
    void updated();

  private:
    explicit Cue(TrackId trackId);
    Cue(int id, TrackId trackId, CueType type, int position, int length,
        int hotCue, QString label);
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

    friend class Track;
    friend class CueDAO;
};

typedef QSharedPointer<Cue> CuePointer;

#endif /* CUE_H */
