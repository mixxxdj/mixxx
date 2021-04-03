#pragma once

#include <QAtomicInt>
#include <QAtomicPointer>
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

    HotcueControl(const QString& group, int hotcueIndex);
    ~HotcueControl() override;

    int getHotcueIndex() const {
        return m_hotcueIndex;
    }

    CuePointer getCue() const {
        return m_pCue;
    }
    void setCue(const CuePointer& pCue);
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

    /// Used for caching the preview state of this hotcue control
    /// for the case the cue is deleted during preview.
    mixxx::CueType getPreviewingType() const {
        return m_previewingType.getValue();
    }

    /// Used for caching the preview state of this hotcue control
    /// for the case the cue is deleted during preview.
    double getPreviewingPosition() const {
        return m_previewingPosition.getValue();
    }

    /// Used for caching the preview state of this hotcue control
    /// for the case the cue is deleted during preview.
    void cachePreviewingStartState() {
        if (m_pCue) {
            m_previewingPosition.setValue(m_pCue->getPosition());
            m_previewingType.setValue(m_pCue->getType());
        } else {
            m_previewingType.setValue(mixxx::CueType::Invalid);
        }
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
    ConfigKey keyForControl(const QString& name);

    const QString m_group;
    const int m_hotcueIndex;
    CuePointer m_pCue;

    // Hotcue state controls
    std::unique_ptr<ControlObject> m_hotcuePosition;
    std::unique_ptr<ControlObject> m_hotcueEndPosition;
    std::unique_ptr<ControlObject> m_pHotcueStatus;
    std::unique_ptr<ControlObject> m_hotcueType;
    std::unique_ptr<ControlObject> m_hotcueColor;
    // Hotcue button controls
    std::unique_ptr<ControlPushButton> m_hotcueSet;
    std::unique_ptr<ControlPushButton> m_hotcueSetCue;
    std::unique_ptr<ControlPushButton> m_hotcueSetLoop;
    std::unique_ptr<ControlPushButton> m_hotcueGoto;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndPlay;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndStop;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndLoop;
    std::unique_ptr<ControlPushButton> m_hotcueCueLoop;
    std::unique_ptr<ControlPushButton> m_hotcueActivate;
    std::unique_ptr<ControlPushButton> m_hotcueActivateCue;
    std::unique_ptr<ControlPushButton> m_hotcueActivateLoop;
    std::unique_ptr<ControlPushButton> m_hotcueActivatePreview;
    std::unique_ptr<ControlPushButton> m_hotcueClear;

    ControlValueAtomic<mixxx::CueType> m_previewingType;
    ControlValueAtomic<double> m_previewingPosition;
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(const QString& group,
            UserSettingsPointer pConfig);
    ~CueControl() override;

    void hintReader(HintVector* pHintList) override;
    bool updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay, bool playPossible);
    void updateIndicators();

    bool isTrackAtIntroCue();
    /// Returns true if the current position is inside the intro.
    /// Returns false if not, or if the intro is not enabled.
    bool inIntro() const;

    /// Returns true if the current position is inside the outro.
    /// Returns false if not, or if the outro is not enabled.
    bool inOutro() const;

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
    void updateCurrentlyPreviewingIndex(int hotcueIndex);
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
    void attachCue(const CuePointer& pCue, HotcueControl* pControl);
    void detachCue(HotcueControl* pControl);
    void setCurrentSavedLoopControlAndActivate(HotcueControl* pControl);
    void loadCuesFromTrack();
    double quantizeCuePoint(double position);
    double getQuantizedCurrentPosition();
    TrackAt getTrackAt() const;
    void seekOnLoad(double seekOnLoadPosition);
    void setHotcueFocusIndex(int hotcueIndex);
    int getHotcueFocusIndex() const;

    UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    QAtomicInt m_currentlyPreviewingIndex;
    ControlObject* m_pPlay;
    ControlObject* m_pStopButton;
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
    ControlObject* m_pPlayLatched;
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

    QAtomicPointer<HotcueControl> m_pCurrentSavedLoopControl;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    // Must be locked when using the m_pLoadedTrack and it's properties
    QMutex m_trackMutex;
    TrackPointer m_pLoadedTrack; // is written from an engine worker thread

    friend class HotcueControlTest;
};
