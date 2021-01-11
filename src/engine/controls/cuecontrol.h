#pragma once

#include <QList>
#include <QMutex>

#include "control/controlproxy.h"
#include "engine/controls/enginecontrol.h"
#include "preferences/colorpalettesettings.h"
#include "preferences/usersettings.h"
#include "track/cue.h"
#include "track/track_decl.h"

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

inline SeekOnLoadMode seekOnLoadModeFromDouble(double value) {
    return static_cast<SeekOnLoadMode>(int(value));
}

class HotcueControl : public QObject {
    Q_OBJECT
  public:
    HotcueControl(const QString& group, int hotcueIndex);
    ~HotcueControl() override;

    int getHotcueIndex() const {
        return m_hotcueIndex;
    }

    CuePointer getCue() const {
        return m_pCue;
    }
    double getPosition() const;
    void setCue(const CuePointer& pCue);
    void resetCue();
    void setPosition(double position);
    void setColor(mixxx::RgbColor::optional_t newColor);
    mixxx::RgbColor::optional_t getColor() const;

    // Used for caching the preview state of this hotcue control.
    bool isPreviewing() const {
        return m_bPreviewing;
    }
    void setPreviewing(bool bPreviewing) {
        m_bPreviewing = bPreviewing;
    }
    double getPreviewingPosition() const {
        return m_previewingPosition;
    }
    void setPreviewingPosition(double position) {
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
    void slotHotcueColorChangeRequest(double newColor);
    void slotHotcueColorChanged(double newColor);

  signals:
    void hotcueSet(HotcueControl* pHotcue, double v);
    void hotcueGoto(HotcueControl* pHotcue, double v);
    void hotcueGotoAndPlay(HotcueControl* pHotcue, double v);
    void hotcueGotoAndStop(HotcueControl* pHotcue, double v);
    void hotcueActivate(HotcueControl* pHotcue, double v);
    void hotcueActivatePreview(HotcueControl* pHotcue, double v);
    void hotcueClear(HotcueControl* pHotcue, double v);
    void hotcuePositionChanged(HotcueControl* pHotcue, double newPosition);
    void hotcueColorChanged(HotcueControl* pHotcue, double newColor);
    void hotcuePlay(double v);

  private:
    ConfigKey keyForControl(const QString& name);

    const QString m_group;
    const int m_hotcueIndex;
    CuePointer m_pCue;

    // Hotcue state controls
    std::unique_ptr<ControlObject> m_hotcuePosition;
    std::unique_ptr<ControlObject> m_hotcueEnabled;
    std::unique_ptr<ControlObject> m_hotcueColor;
    // Hotcue button controls
    std::unique_ptr<ControlPushButton> m_hotcueSet;
    std::unique_ptr<ControlPushButton> m_hotcueGoto;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndPlay;
    std::unique_ptr<ControlPushButton> m_hotcueGotoAndStop;
    std::unique_ptr<ControlPushButton> m_hotcueActivate;
    std::unique_ptr<ControlPushButton> m_hotcueActivatePreview;
    std::unique_ptr<ControlPushButton> m_hotcueClear;

    bool m_bPreviewing;
    double m_previewingPosition;
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
    void resetIndicators();
    bool isPlayingByPlayButton();
    bool getPlayFlashingAtPause();
    SeekOnLoadMode getSeekOnLoadPreference();
    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

  private slots:
    void quantizeChanged(double v);

    void cueUpdated();
    void trackAnalyzed();
    void trackCuesUpdated();
    void hotcueSet(HotcueControl* pControl, double v);
    void hotcueGoto(HotcueControl* pControl, double v);
    void hotcueGotoAndPlay(HotcueControl* pControl, double v);
    void hotcueGotoAndStop(HotcueControl* pControl, double v);
    void hotcueActivate(HotcueControl* pControl, double v);
    void hotcueActivatePreview(HotcueControl* pControl, double v);
    void hotcueClear(HotcueControl* pControl, double v);
    void hotcuePositionChanged(HotcueControl* pControl, double newPosition);

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
    void loadCuesFromTrack();
    double quantizeCuePoint(double position);
    double getQuantizedCurrentPosition();
    TrackAt getTrackAt() const;
    void seekOnLoad(double seekOnLoadPosition);
    void setHotcueFocusIndex(int hotcueIndex);
    int getHotcueFocusIndex() const;

    UserSettingsPointer m_pConfig;
    ColorPaletteSettings m_colorPaletteSettings;
    bool m_bPreviewing;
    ControlObject* m_pPlay;
    ControlObject* m_pStopButton;
    int m_iCurrentlyPreviewingHotcues;
    ControlObject* m_pQuantizeEnabled;
    ControlObject* m_pClosestBeat;
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

    TrackPointer m_pLoadedTrack; // is written from an engine worker thread

    // Tells us which controls map to which hotcue
    QMap<QObject*, int> m_controlMap;

    QMutex m_mutex;
};
