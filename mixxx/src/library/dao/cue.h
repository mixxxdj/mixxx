// cue.h
// Created 10/26/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUE_H
#define CUE_H

#include <QObject>
#include <QMutex>

class CueDAO;
class TrackInfoObject;

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

    bool isDirty();
    int getId();
    int getTrackId();

    CueType getType();
    void setType(CueType type);

    int getPosition();
    void setPosition(int position);

    int getLength();
    void setLength(int length);

    int getHotCue();
    void setHotCue(int hotCue);

    const QString getLabel();
    void setLabel(const QString label);

  signals:
    void updated();

  private:
    Cue(int trackId);
    Cue(int id, int trackId, CueType type, int position, int length,
        int hotCue, QString label);
    void setDirty(bool dirty);
    void setId(int id);
    void setTrackId(int trackId);

    bool m_bDirty;
    bool m_bDelete;
    int m_iId;
    int m_iTrackId;
    CueType m_type;
    int m_iPosition;
    int m_iLength;
    int m_iHotCue;
    QString m_label;
    QMutex m_mutex;

    friend class TrackInfoObject;
    friend class CueDAO;
};

#endif /* CUE_H */
