// cuecontrol.h
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUECONTROL_H
#define CUECONTROL_H

#include <QList>
#include <QMutex>

#include "engine/enginecontrol.h"
#include "configobject.h"
#include "trackinfoobject.h"

#define NUM_HOT_CUES 37

class ControlObject;
class ControlPushButton;
class Cue;

class HotcueControl : public QObject {
    Q_OBJECT
  public:
    HotcueControl(const char* pGroup, int hotcueNumber);
    virtual ~HotcueControl();

    inline int getHotcueNumber() { return m_iHotcueNumber; }
    inline Cue* getCue() { return m_pCue; }
    inline void setCue(Cue* pCue) { m_pCue = pCue; }
    inline ControlObject* getPosition() { return m_hotcuePosition; }
    inline ControlObject* getEnabled() { return m_hotcueEnabled; }

    // Used for caching the preview state of this hotcue control.
    inline bool isPreviewing() { return m_bPreviewing; }
    inline void setPreviewing(bool bPreviewing) { m_bPreviewing = bPreviewing; }
    inline int getPreviewingPosition() { return m_iPreviewingPosition; }
    inline void setPreviewingPosition(int iPosition) { m_iPreviewingPosition = iPosition; }

  private slots:
    void slotHotcueSet(double v);
    void slotHotcueGoto(double v);
    void slotHotcueGotoAndStop(double v);
    void slotHotcueActivate(double v);
    void slotHotcueActivatePreview(double v);
    void slotHotcueClear(double v);
    void slotHotcuePositionChanged(double newPosition);

  signals:
    void hotcueSet(HotcueControl* pHotcue, double v);
    void hotcueGoto(HotcueControl* pHotcue, double v);
    void hotcueGotoAndStop(HotcueControl* pHotcue, double v);
    void hotcueActivate(HotcueControl* pHotcue, double v);
    void hotcueActivatePreview(HotcueControl* pHotcue, double v);
    void hotcueClear(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcuePlay(double v);

  private:
    ConfigKey keyForControl(int hotcue, QString name);

    const char* m_pGroup;
    int m_iHotcueNumber;
    Cue* m_pCue;

    // Hotcue state controls
    ControlObject* m_hotcuePosition;
    ControlObject* m_hotcueEnabled;
    // Hotcue button controls
    ControlObject* m_hotcueSet;
    ControlObject* m_hotcueGoto;
    ControlObject* m_hotcueGotoAndStop;
    ControlObject* m_hotcueActivate;
    ControlObject* m_hotcueActivatePreview;
    ControlObject* m_hotcueClear;

    bool m_bPreviewing;
    int m_iPreviewingPosition;
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(const char * _group,
               ConfigObject<ConfigValue> * _config);
    virtual ~CueControl();

    virtual void hintReader(QList<Hint>& hintList);

  public slots:
    void loadTrack(TrackPointer pTrack);
    void unloadTrack(TrackPointer pTrack);

  private slots:
    void cueUpdated();
    void trackCuesUpdated();
    void hotcueSet(HotcueControl* pControl, double v);
    void hotcueGoto(HotcueControl* pControl, double v);
    void hotcueGotoAndStop(HotcueControl* pControl, double v);
    void hotcueActivate(HotcueControl* pControl, double v);
    void hotcueActivatePreview(HotcueControl* pControl, double v);
    void hotcueClear(HotcueControl* pControl, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);

    void cueSet(double v);
    void cueGoto(double v);
    void cueGotoAndStop(double v);
    void cueSimple(double v);
    void cuePreview(double v);
    void cueCDJ(double v);
    void cueDefault(double v);
    void cuePlay(double v);

  private:
    // These methods are not thread safe, only call them when the lock is held.
    void createControls();
    void attachCue(Cue* pCue, int hotcueNumber);
    void detachCue(int hotcueNumber);
    void saveCuePoint(double cuePoint);

    bool m_bHotcueCancel;
    bool m_bPreviewing;
    bool m_bPreviewingHotcue;
    ControlObject* m_pPlayButton;
    int m_iCurrentlyPreviewingHotcues;
    ControlObject* m_pQuantizeEnabled;
    ControlObject* m_pNextBeat;
    ControlObject* m_pClosestBeat;

    const int m_iNumHotCues;
    QList<HotcueControl*> m_hotcueControl;

    ControlObject* m_pTrackSamples;
    ControlObject* m_pCuePoint;
    ControlObject* m_pCueMode;
    ControlPushButton* m_pCueSet;
    ControlPushButton* m_pCueSimple;
    ControlPushButton* m_pCueCDJ;
    ControlPushButton* m_pCueDefault;
    ControlPushButton* m_pCueGoto;
    ControlPushButton* m_pCueGotoAndStop;
    ControlPushButton* m_pCuePreview;

    TrackPointer m_pLoadedTrack;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    QMutex m_mutex;
};


#endif /* CUECONTROL_H */
