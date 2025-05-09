#pragma once

#include <gtest/gtest_prod.h>

#include <QAtomicInt>
#include <QAtomicPointer>
#include <QList>

#include "engine/controls/enginecontrol.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/usersettings.h"
#include "track/cue.h"
#include "track/track_decl.h"
#include "util/compatibility/qmutex.h"
#include "util/parented_ptr.h"

class ControlObject;
class ControlPushButton;
class ControlIndicator;
class ControlProxy;

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
    FirstHotcue = 4,
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
        /// controller mappings to highlight the cue control that has saved the current loop,
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

    mixxx::audio::FramePos getPosition() const;
    void setPosition(mixxx::audio::FramePos position);

    mixxx::audio::FramePos getEndPosition() const;
    void setEndPosition(mixxx::audio::FramePos endPosition);

    mixxx::CueType getType() const;
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
    mixxx::audio::FramePos getPreviewingPosition() const {
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
    void slotHotcueSwap(double v);
    void slotHotcueEndPositionChanged(double newPosition);
    void slotHotcuePositionChanged(double newPosition);
    void slotHotcueColorChangeRequest(double newColor);

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
    void hotcueSwap(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcueEndPositionChanged(HotcueControl* pHotcue, double newEndPosition);
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
    std::unique_ptr<ControlPushButton> m_hotcueSwap;

    ControlValueAtomic<mixxx::CueType> m_previewingType;
    ControlValueAtomic<mixxx::audio::FramePos> m_previewingPosition;
};

class CueControl : public EngineControl {
    Q_OBJECT
  public:
    CueControl(const QString& group,
            UserSettingsPointer pConfig);
    ~CueControl() override;

    void hintReader(gsl::not_null<HintVector*> pHintList) override;
    bool updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay, bool playPossible);
    void updateIndicators();
    bool isTrackAtIntroCue();
    void resetIndicators();
    bool isPlayingByPlayButton();
    bool getPlayFlashingAtPause();
    SeekOnLoadMode getSeekOnLoadPreference();
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  signals:
    void loopRemove();

  public slots:
    void slotLoopReset();
    void slotLoopEnabledChanged(bool enabled);
    void slotLoopUpdated(mixxx::audio::FramePos startPosition, mixxx::audio::FramePos endPosition);

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
    void hotcueSwap(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);
    void hotcueEndPositionChanged(HotcueControl* pControl, double newEndPosition);

    void hotcueFocusColorNext(double v);
    void hotcueFocusColorPrev(double v);

    void passthroughChanged(double v);

    void setHotcueIndicesSortedByPosition(double v);
    void setHotcueIndicesSortedByPositionCompress(double v);

    void cueSet(double v);
    void cueClear(double v);
    void cueGoto(double v);
    void cueGotoAndPlay(double v);
    void cueGotoAndStop(double v);
    void cuePreview(double v);
    FRIEND_TEST(CueControlTest, SeekOnSetCueCDJ);
    void cueCDJ(double v);
    void cueDenon(double v);
    FRIEND_TEST(CueControlTest, SeekOnSetCuePlay);
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
    void connectControls();
    void disconnectControls();

    void attachCue(const CuePointer& pCue, HotcueControl* pControl);
    void detachCue(HotcueControl* pControl);
    void setCurrentSavedLoopControlAndActivate(HotcueControl* pControl);
    void loadCuesFromTrack();
    mixxx::audio::FramePos quantizeCuePoint(mixxx::audio::FramePos position);
    mixxx::audio::FramePos getQuantizedCurrentPosition();
    TrackAt getTrackAt() const;
    void seekOnLoad(mixxx::audio::FramePos seekOnLoadPosition);
    void setHotcueFocusIndex(int hotcueIndex);
    int getHotcueFocusIndex() const;
    mixxx::RgbColor colorFromConfig(const ConfigKey& configKey);

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
    ControlValueAtomic<mixxx::audio::FramePos> m_usedSeekOnLoadPosition;

    QList<HotcueControl*> m_hotcueControls;

    ControlObject* m_pTrackSamples;
    ControlObject* m_pCuePoint;
    ControlObject* m_pCueMode;
    std::unique_ptr<ControlPushButton> m_pCueSet;
    std::unique_ptr<ControlPushButton> m_pCueClear;
    std::unique_ptr<ControlPushButton> m_pCueCDJ;
    std::unique_ptr<ControlPushButton> m_pCueDefault;
    std::unique_ptr<ControlPushButton> m_pPlayStutter;
    std::unique_ptr<ControlIndicator> m_pCueIndicator;
    std::unique_ptr<ControlIndicator> m_pPlayIndicator;
    std::unique_ptr<ControlObject> m_pPlayLatched;
    std::unique_ptr<ControlPushButton> m_pCueGoto;
    std::unique_ptr<ControlPushButton> m_pCueGotoAndPlay;
    std::unique_ptr<ControlPushButton> m_pCuePlay;
    std::unique_ptr<ControlPushButton> m_pCueGotoAndStop;
    std::unique_ptr<ControlPushButton> m_pCuePreview;

    std::unique_ptr<ControlObject> m_pIntroStartPosition;
    std::unique_ptr<ControlObject> m_pIntroStartEnabled;
    std::unique_ptr<ControlPushButton> m_pIntroStartSet;
    std::unique_ptr<ControlPushButton> m_pIntroStartClear;
    std::unique_ptr<ControlPushButton> m_pIntroStartActivate;

    std::unique_ptr<ControlObject> m_pIntroEndPosition;
    std::unique_ptr<ControlObject> m_pIntroEndEnabled;
    std::unique_ptr<ControlPushButton> m_pIntroEndSet;
    std::unique_ptr<ControlPushButton> m_pIntroEndClear;
    std::unique_ptr<ControlPushButton> m_pIntroEndActivate;

    std::unique_ptr<ControlObject> m_pOutroStartPosition;
    std::unique_ptr<ControlObject> m_pOutroStartEnabled;
    std::unique_ptr<ControlPushButton> m_pOutroStartSet;
    std::unique_ptr<ControlPushButton> m_pOutroStartClear;
    std::unique_ptr<ControlPushButton> m_pOutroStartActivate;

    std::unique_ptr<ControlObject> m_pOutroEndPosition;
    std::unique_ptr<ControlObject> m_pOutroEndEnabled;
    std::unique_ptr<ControlPushButton> m_pOutroEndSet;
    std::unique_ptr<ControlPushButton> m_pOutroEndClear;
    std::unique_ptr<ControlPushButton> m_pOutroEndActivate;

    ControlValueAtomic<double> m_n60dBSoundStartPosition;

    std::unique_ptr<ControlProxy> m_pVinylControlEnabled;
    std::unique_ptr<ControlProxy> m_pVinylControlMode;

    std::unique_ptr<ControlObject> m_pHotcueFocus;
    std::unique_ptr<ControlPushButton> m_pHotcueFocusColorNext;
    std::unique_ptr<ControlPushButton> m_pHotcueFocusColorPrev;

    parented_ptr<ControlProxy> m_pPassthrough;

    std::unique_ptr<ControlPushButton> m_pSortHotcuesByPos;
    std::unique_ptr<ControlPushButton> m_pSortHotcuesByPosCompress;

    QAtomicPointer<HotcueControl> m_pCurrentSavedLoopControl;

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    // Must be locked when using the m_pLoadedTrack and it's properties
    QT_RECURSIVE_MUTEX m_trackMutex;
    TrackPointer m_pLoadedTrack; // is written from an engine worker thread

    friend class HotcueControlTest;
};
