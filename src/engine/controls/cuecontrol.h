// cuecontrol.h
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#ifndef CUECONTROL_H
#define CUECONTROL_H

#include <QList>
#include <QMutex>

#include "control/controlproxy.h"
#include "engine/controls/enginecontrol.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/usersettings.h"
#include "track/cue.h"
#include "track/track_decl.h"
#include "util/parented_ptr.h"

#define NUM_HOT_CUES 37

class ControlObject;
class ControlPushButton;
class ControlIndicator;

enum class CueMode {
    Mixxx,
    Pioneer,
    Denon,
    Numark,
    MixxxNoBlinking,
    CueAndPlay
};

enum class SeekOnLoadMode {
    MainCue = 0,    // Use main cue point
    Beginning = 1,  // Use 0:00.000
    FirstSound = 2, // Skip leading silence
    IntroStart = 3, // Use intro start cue point
};

/// Used for requesting a specific hotcue type when activating/setting a
/// hotcue. Auto will make CueControl determine the type automatically (i.e.
/// create a loop cue if a loop is set, and a regular cue in all other cases).
enum class HotcueSetMode {
    Auto = 0,
    Cue = 1,
    Loop = 2,
};

inline SeekOnLoadMode seekOnLoadModeFromDouble(double value) {
    return static_cast<SeekOnLoadMode>(int(value));
}

/// A `HotcueControl` represents a hotcue slot. It can either be empty or have
/// a (hot-)cue attached to it.
///
/// TODO(XXX): This class should be moved into a separate file.
class HotcueControl : public QObject {
    Q_OBJECT
  public:
    /// Describes the current status of the hotcue
    enum class Status {
        /// Hotuce not set
        Empty = 0,
        /// Hotcue is set and can be used
        Set = 1,
        /// Hotcue is currently active (this only applies to Saved Loop cues
        /// while their loop is enabled). This status can be used by skins or
        /// controller mappings to highlight a the cue control that has saved the current loop,
        /// because resizing or moving the loop will make persistent changes to
        /// the cue.
        Active = 2,
    };

    HotcueControl(QString group, int hotcueNumber);
    ~HotcueControl() override;

    int getHotcueNumber() const {
        return m_iHotcueNumber;
    }

    CuePointer getCue() const {
        return m_pCue;
    }
    void setCue(CuePointer pCue);
    void resetCue();

    double getPosition() const;
    void setPosition(double position);

    double getEndPosition() const;
    void setEndPosition(double endPosition);

    void setType(mixxx::CueType type);

    void setStatus(HotcueControl::Status status);
    HotcueControl::Status getStatus() const;

    void setColor(mixxx::RgbColor::optional_t newColor);
    mixxx::RgbColor::optional_t getColor() const;

    // Used for caching the preview state of this hotcue control.
    mixxx::CueType getPreviewingType() const {
        return m_previewingType;
    }
    void setPreviewingType(mixxx::CueType type) {
        m_previewingType = type;
    }
    double getPreviewingPosition() const {
        return m_previewingPosition;
    }
    void setPreviewingPosition(double position) {
        m_previewingPosition = position;
    }

  private slots:
    void slotHotcueSet(double v);
    void slotHotcueSetCue(double v);
    void slotHotcueSetLoop(double v);
    void slotHotcueGoto(double v);
    void slotHotcueGotoAndPlay(double v);
    void slotHotcueGotoAndStop(double v);
    void slotHotcueGotoAndLoop(double v);
    void slotHotcueCueLoop(double v);
    void slotHotcueActivate(double v);
    void slotHotcueActivateCue(double v);
    void slotHotcueActivateLoop(double v);
    void slotHotcueActivatePreview(double v);
    void slotHotcueClear(double v);
    void slotHotcueEndPositionChanged(double newPosition);
    void slotHotcuePositionChanged(double newPosition);
    void slotHotcueColorChangeRequest(double newColor);
    void slotHotcueColorChanged(double newColor);

  signals:
    void hotcueSet(HotcueControl* pHotcue, double v, HotcueSetMode mode);
    void hotcueGoto(HotcueControl* pHotcue, double v);
    void hotcueGotoAndPlay(HotcueControl* pHotcue, double v);
    void hotcueGotoAndStop(HotcueControl* pHotcue, double v);
    void hotcueGotoAndLoop(HotcueControl* pHotcue, double v);
    void hotcueCueLoop(HotcueControl* pHotcue, double v);
    void hotcueActivate(HotcueControl* pHotcue, double v, HotcueSetMode mode);
    void hotcueActivatePreview(HotcueControl* pHotcue, double v);
    void hotcueClear(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcueEndPositionChanged(HotcueControl* pHotcue, double newEndPosition);
    void hotcueColorChanged(HotcueControl* pHotcue, double newColor);
    void hotcuePlay(double v);

  private:
    ConfigKey keyForControl(int hotcue, const char* name);

    const QString m_group;
    int m_iHotcueNumber;
    CuePointer m_pCue;

    // Hotcue state controls
    ControlObject* m_hotcuePosition;
    ControlObject* m_hotcueEndPosition;
    ControlObject* m_pHotcueStatus;
    ControlObject* m_hotcueType;
    ControlObject* m_hotcueColor;
    // Hotcue button controls
    ControlObject* m_hotcueSet;
    ControlObject* m_hotcueSetCue;
    ControlObject* m_hotcueSetLoop;
    ControlObject* m_hotcueGoto;
    ControlObject* m_hotcueGotoAndPlay;
    ControlObject* m_hotcueGotoAndStop;
    ControlObject* m_hotcueGotoAndLoop;
    ControlObject* m_hotcueCueLoop;
    ControlObject* m_hotcueActivate;
    ControlObject* m_hotcueActivateCue;
    ControlObject* m_hotcueActivateLoop;
    ControlObject* m_hotcueActivatePreview;
    ControlObject* m_hotcueClear;

    mixxx::CueType m_previewingType;
    double m_previewingPosition;
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(QString group,
               UserSettingsPointer pConfig);
    ~CueControl() override;

    void hintReader(HintVector* pHintList) override;
    bool updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay, bool playPossible);
    void updateIndicators();
    bool isTrackAtIntroCue();
    void resetIndicators();
    bool isPlayingByPlayButton();
    bool getPlayFlashingAtPause();
    SeekOnLoadMode getSeekOnLoadPreference();
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  public slots:
    void slotLoopReset();
    void slotLoopEnabledChanged(bool enabled);
    void slotLoopUpdated(double startPosition, double endPosition);

  private slots:
    void quantizeChanged(double v);

    void cueUpdated();
    void trackAnalyzed();
    void trackCuesUpdated();
    void hotcueSet(HotcueControl* pControl, double v, HotcueSetMode mode);
    void hotcueGoto(HotcueControl* pControl, double v);
    void hotcueGotoAndPlay(HotcueControl* pControl, double v);
    void hotcueGotoAndStop(HotcueControl* pControl, double v);
    void hotcueGotoAndLoop(HotcueControl* pControl, double v);
    void hotcueCueLoop(HotcueControl* pControl, double v);
    void hotcueActivate(HotcueControl* pControl, double v, HotcueSetMode mode);
    void hotcueActivatePreview(HotcueControl* pControl, double v);
    void hotcueClear(HotcueControl* pControl, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);
    void hotcueEndPositionChanged(HotcueControl* pControl, double newEndPosition);

    void hotcueFocusColorNext(double v);
    void hotcueFocusColorPrev(double v);

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
    enum class TrackAt {
        Cue,
        End,
        ElseWhere
    };

    // These methods are not thread safe, only call them when the lock is held.
    void createControls();
    void attachCue(CuePointer pCue, HotcueControl* pControl);
    void detachCue(HotcueControl* pControl);
    void setCurrentSavedLoopControlAndActivate(HotcueControl* pControl);
    void loadCuesFromTrack();
    double quantizeCuePoint(double position);
    double getQuantizedCurrentPosition();
    TrackAt getTrackAt() const;
    void seekOnLoad(double seekOnLoadPosition);

    UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    bool m_bPreviewing;
    ControlObject* m_pPlay;
    ControlObject* m_pStopButton;
    int m_iCurrentlyPreviewingHotcues;
    ControlObject* m_pQuantizeEnabled;
    ControlObject* m_pClosestBeat;
    parented_ptr<ControlProxy> m_pLoopStartPosition;
    parented_ptr<ControlProxy> m_pLoopEndPosition;
    parented_ptr<ControlProxy> m_pLoopEnabled;
    parented_ptr<ControlProxy> m_pBeatLoopActivate;
    parented_ptr<ControlProxy> m_pBeatLoopSize;
    bool m_bypassCueSetByPlay;
    ControlValueAtomic<double> m_usedSeekOnLoadPosition;

    const int m_iNumHotCues;
    QList<HotcueControl*> m_hotcueControls;

    ControlObject* m_pTrackSamples;
    ControlObject* m_pCuePoint;
    ControlObject* m_pCueMode;
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

    ControlObject* m_pHotcueFocus;
    ControlObject* m_pHotcueFocusColorNext;
    ControlObject* m_pHotcueFocusColorPrev;

    TrackPointer m_pLoadedTrack; // is written from an engine worker thread
    HotcueControl* m_pCurrentSavedLoopControl;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    // TODO(daschuer): It looks like the whole m_mutex is broken. Originally it
    // ensured that the main cue really belongs to the loaded track. Now that
    // we have hot cues that are altered outsite this guard this guarantee has
    // become void.
    //
    // We have multiple cases where it locks m_pLoadedTrack and
    // pControl->getCue(). This guards the hotcueClear() that could detach the
    // cue call, but doesn't protect from cue changes via loadCuesFromTrack()
    // which is called outside the mutex lock.
    //
    // We need to repair this.
    QMutex m_mutex;

    friend class HotcueControlTest;
};


#endif /* CUECONTROL_H */
