// cuecontrol.h
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUECONTROL_H
#define CUECONTROL_H

#include <QList>
#include <QMutex>

#include "engine/controls/enginecontrol.h"
#include "preferences/usersettings.h"
#include "control/controlproxy.h"
#include "track/track.h"

#define NUM_HOT_CUES 37

class ControlObject;
class ControlPushButton;
class ControlIndicator;

enum SeekOnLoadMode {
    SEEK_ON_LOAD_DEFAULT = 0,  // Use CueRecall preference setting
    SEEK_ON_LOAD_ZERO_POS = 1,  // Use 0:00.000
    SEEK_ON_LOAD_MAIN_CUE = 2,  // Use main cue point
    SEEK_ON_LOAD_INTRO_CUE = 3,  // Use intro cue point
    SEEK_ON_LOAD_NUM_MODES
};

inline SeekOnLoadMode seekOnLoadModeFromDouble(double value) {
    // msvs does not allow to cast from double to an enum
    SeekOnLoadMode mode = static_cast<SeekOnLoadMode>(int(value));
    if (mode >= SEEK_ON_LOAD_NUM_MODES || mode < 0) {
        return SEEK_ON_LOAD_DEFAULT;
    }
    return mode;
}

class HotcueControl : public QObject {
    Q_OBJECT
  public:
    HotcueControl(QString group, int hotcueNumber);
    virtual ~HotcueControl();

    inline int getHotcueNumber() { return m_iHotcueNumber; }
    inline CuePointer getCue() { return m_pCue; }
    double getPosition() const;
    void setCue(CuePointer pCue);
    void resetCue();
    void setPosition(double position);
    void setColor(PredefinedColorPointer newColor);
    PredefinedColorPointer getColor() const;

    // Used for caching the preview state of this hotcue control.
    inline bool isPreviewing() {
        return m_bPreviewing;
    }
    inline void setPreviewing(bool bPreviewing) {
        m_bPreviewing = bPreviewing;
    }
    inline double getPreviewingPosition() {
        return m_previewingPosition;
    }
    inline void setPreviewingPosition(double position) {
        m_previewingPosition = position;
    }

  private slots:
    void slotHotcueSet(double v);
    void slotHotcueGoto(double v);
    void slotHotcueGotoAndPlay(double v);
    void slotHotcueGotoAndStop(double v);
    void slotHotcueActivate(double v);
    void slotHotcueActivatePreview(double v);
    void slotHotcueClear(double v);
    void slotHotcuePositionChanged(double newPosition);
    void slotHotcueColorChanged(double newColorId);

  signals:
    void hotcueSet(HotcueControl* pHotcue, double v);
    void hotcueGoto(HotcueControl* pHotcue, double v);
    void hotcueGotoAndPlay(HotcueControl* pHotcue, double v);
    void hotcueGotoAndStop(HotcueControl* pHotcue, double v);
    void hotcueActivate(HotcueControl* pHotcue, double v);
    void hotcueActivatePreview(HotcueControl* pHotcue, double v);
    void hotcueClear(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcueColorChanged(HotcueControl* pHotcue, double newColorId);
    void hotcuePlay(double v);

  private:
    ConfigKey keyForControl(int hotcue, const char* name);

    QString m_group;
    int m_iHotcueNumber;
    CuePointer m_pCue;

    // Hotcue state controls
    ControlObject* m_hotcuePosition;
    ControlObject* m_hotcueEnabled;
    ControlObject* m_hotcueColor;
    // Hotcue button controls
    ControlObject* m_hotcueSet;
    ControlObject* m_hotcueGoto;
    ControlObject* m_hotcueGotoAndPlay;
    ControlObject* m_hotcueGotoAndStop;
    ControlObject* m_hotcueActivate;
    ControlObject* m_hotcueActivatePreview;
    ControlObject* m_hotcueClear;

    bool m_bPreviewing;
    double m_previewingPosition;
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(QString group,
               UserSettingsPointer pConfig);
    ~CueControl() override;

    virtual void hintReader(HintVector* pHintList) override;
    bool updateIndicatorsAndModifyPlay(bool newPlay, bool playPossible);
    void updateIndicators();
    bool isTrackAtZeroPos();
    bool isTrackAtIntroCue();
    void resetIndicators();
    bool isPlayingByPlayButton();
    bool getPlayFlashingAtPause();
    bool isCueRecallEnabled();
    void trackLoaded(TrackPointer pNewTrack) override;
    SeekOnLoadMode getSeekOnLoadMode();

  private slots:
    void quantizeChanged(double v);

    void cueUpdated();
    void trackCuesUpdated();
    void trackBeatsUpdated();
    void hotcueSet(HotcueControl* pControl, double v);
    void hotcueGoto(HotcueControl* pControl, double v);
    void hotcueGotoAndPlay(HotcueControl* pControl, double v);
    void hotcueGotoAndStop(HotcueControl* pControl, double v);
    void hotcueActivate(HotcueControl* pControl, double v);
    void hotcueActivatePreview(HotcueControl* pControl, double v);
    void hotcueClear(HotcueControl* pControl, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);

    void cueSet(double v);
    void cueClear(double v);
    void cueGoto(double v);
    void cueGotoAndPlay(double v);
    void cueGotoAndStop(double v);
    void cuePreview(double v);
    void cueCDJ(double v);
    void cueDenon(double v);
    void cuePlay(double v);
    void cueDefault(double v);
    void pause(double v);
    void playStutter(double v);

    void introStartSet(double v);
    void introStartClear(double v);
    void introStartActivate(double v);
    void introEndSet(double v);
    void introEndClear(double v);
    void introEndActivate(double v);
    void outroStartSet(double v);
    void outroStartClear(double v);
    void outroStartActivate(double v);
    void outroEndSet(double v);
    void outroEndClear(double v);
    void outroEndActivate(double v);

  private:
    enum class QuantizeMode {
        ClosestBeat,
        PreviousBeat,
        NextBeat,
    };

    enum class TrackAt {
        Cue,
        End,
        ElseWhere
    };

    // These methods are not thread safe, only call them when the lock is held.
    void createControls();
    void attachCue(CuePointer pCue, int hotcueNumber);
    void detachCue(int hotcueNumber);
    void loadCuesFromTrack();
    void reloadCuesFromTrack();
    double quantizeCuePoint(double position, Cue::CueSource source, QuantizeMode mode);
    double quantizeCurrentPosition(QuantizeMode mode);
    TrackAt getTrackAt() const;

    bool m_bPreviewing;
    ControlObject* m_pPlay;
    ControlObject* m_pStopButton;
    int m_iCurrentlyPreviewingHotcues;
    ControlObject* m_pQuantizeEnabled;
    ControlObject* m_pPrevBeat;
    ControlObject* m_pNextBeat;
    ControlObject* m_pClosestBeat;
    bool m_bypassCueSetByPlay;

    const int m_iNumHotCues;
    QList<HotcueControl*> m_hotcueControls;

    ControlObject* m_pTrackSamples;
    ControlObject* m_pCuePoint;
    ControlObject* m_pCueMode;
    ControlObject* m_pSeekOnLoadMode;
    ControlPushButton* m_pCueSet;
    ControlPushButton* m_pCueClear;
    ControlPushButton* m_pCueCDJ;
    ControlPushButton* m_pCueDefault;
    ControlPushButton* m_pPlayStutter;
    ControlIndicator* m_pCueIndicator;
    ControlIndicator* m_pPlayIndicator;
    ControlPushButton* m_pCueGoto;
    ControlPushButton* m_pCueGotoAndPlay;
    ControlPushButton* m_pCuePlay;
    ControlPushButton* m_pCueGotoAndStop;
    ControlPushButton* m_pCuePreview;

    ControlObject* m_pIntroStartPosition;
    ControlObject* m_pIntroStartEnabled;
    ControlPushButton* m_pIntroStartSet;
    ControlPushButton* m_pIntroStartClear;
    ControlPushButton* m_pIntroStartActivate;

    ControlObject* m_pIntroEndPosition;
    ControlObject* m_pIntroEndEnabled;
    ControlPushButton* m_pIntroEndSet;
    ControlPushButton* m_pIntroEndClear;
    ControlPushButton* m_pIntroEndActivate;

    ControlObject* m_pOutroStartPosition;
    ControlObject* m_pOutroStartEnabled;
    ControlPushButton* m_pOutroStartSet;
    ControlPushButton* m_pOutroStartClear;
    ControlPushButton* m_pOutroStartActivate;

    ControlObject* m_pOutroEndPosition;
    ControlObject* m_pOutroEndEnabled;
    ControlPushButton* m_pOutroEndSet;
    ControlPushButton* m_pOutroEndClear;
    ControlPushButton* m_pOutroEndActivate;

    ControlProxy* m_pVinylControlEnabled;
    ControlProxy* m_pVinylControlMode;

    TrackPointer m_pLoadedTrack; // is written from an engine worker thread

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    QMutex m_mutex;
};


#endif /* CUECONTROL_H */
