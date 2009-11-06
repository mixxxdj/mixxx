// cuecontrol.h
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUECONTROL_H
#define CUECONTROL_H

#include <QList>
#include <QMutex>

#include "engine/enginecontrol.h"
#include "configobject.h"

class ControlObject;
class Cue;
class TrackInfoObject;

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(const char * _group,
               const ConfigObject<ConfigValue> * _config);
    virtual ~CueControl();

    virtual void hintReader(QList<Hint>& hintList);

  public slots:
    void loadTrack(TrackInfoObject* pTrack);
    void unloadTrack(TrackInfoObject* pTrack);

  private slots:
    void cueUpdated();
    void trackCuesUpdated();
    void hotcueSet(double v);
    void hotcueGoto(double v);
    void hotcueGotoAndStop(double v);
    void hotcueActivate(double v);
    void hotcueActivatePreview(double v);
    void hotcueClear(double v);

  private:
    ConfigKey keyForControl(int hotcue, QString name);
    void createControls();
    void attachCue(Cue* pCue, int hotKey);
    void detachCue(int hotKey);
    int senderHotcue(QObject* pSender);

    bool m_bPreviewing;
    ControlObject* m_pPlayButton;

    const int m_iNumHotCues;
    // Hotcue state controls
    QList<ControlObject*> m_hotcuePosition;
    QList<ControlObject*> m_hotcueEnabled;
    // Hotcue button controls
    QList<ControlObject*> m_hotcueSet;
    QList<ControlObject*> m_hotcueGoto;
    QList<ControlObject*> m_hotcueGotoAndStop;
    QList<ControlObject*> m_hotcueActivate;
    QList<ControlObject*> m_hotcueActivatePreview;
    QList<ControlObject*> m_hotcueClear;
    QList<Cue*> m_hotcue;

    TrackInfoObject* m_pLoadedTrack;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    QMutex m_mutex;
};


#endif /* CUECONTROL_H */
