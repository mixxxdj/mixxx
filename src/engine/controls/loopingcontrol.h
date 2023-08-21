#pragma once

#include <QObject>
#include <QStack>

#include "control/controlvalue.h"
#include "engine/controls/enginecontrol.h"
#include "engine/controls/ratecontrol.h"
#include "preferences/usersettings.h"
#include "track/beats.h"
#include "track/cue.h"
#include "track/track_decl.h"

class ControlPushButton;
class ControlObject;

class LoopMoveControl;
class BeatJumpControl;
class BeatLoopingControl;

class LoopingControl : public EngineControl {
    Q_OBJECT
  public:
    static QList<double> getBeatSizes();

    LoopingControl(const QString& group, UserSettingsPointer pConfig);
    ~LoopingControl() override;

    // process() updates the internal state of the LoopingControl to reflect the
    // correct current sample. If a loop should be taken LoopingControl returns
    // the sample that should be seeked to. Otherwise it returns currentPosition.
    void process(const double dRate,
            mixxx::audio::FramePos currentPosition,
            const int iBufferSize) override;

    // nextTrigger returns the sample at which the engine will be triggered to
    // take a loop, given the value of currentPosition and the playback direction.
    virtual mixxx::audio::FramePos nextTrigger(bool reverse,
            mixxx::audio::FramePos currentPosition,
            mixxx::audio::FramePos* pTargetPosition);

    // hintReader will add to hintList hints both the loop in and loop out
    // sample, if set.
    void hintReader(gsl::not_null<HintVector*> pHintList) override;
    mixxx::audio::FramePos getSyncPositionInsideLoop(
            mixxx::audio::FramePos requestedPlayPosition,
            mixxx::audio::FramePos syncedPlayPosition);

    void notifySeek(mixxx::audio::FramePos position) override;

    // Wrapper to use adjustedPositionInsideAdjustedLoop() with the current loop.
    // Called from EngineBuffer while slip mode is enabled
    mixxx::audio::FramePos adjustedPositionForCurrentLoop(
            mixxx::audio::FramePos newPosition,
            bool reverse);

    void setBeatLoop(mixxx::audio::FramePos startPosition, bool enabled);
    void setLoop(mixxx::audio::FramePos startPosition,
            mixxx::audio::FramePos endPosition,
            bool enabled);
    mixxx::audio::FramePos getLoopStartPosition() {
        return m_loopInfo.getValue().startPosition;
    }
    mixxx::audio::FramePos getLoopEndPosition() {
        return m_loopInfo.getValue().endPosition;
    }
    void setRateControl(RateControl* rateControl);

    bool isLoopingEnabled() {
        return m_bLoopingEnabled;
    }
    bool isLoopRollActive() {
        return m_bLoopRollActive;
    }
    bool loopWasEnabledBeforeSlipEnable() {
        return m_bLoopWasEnabledBeforeSlipEnable;
    }

    void trackLoaded(TrackPointer pNewTrack) override;
    void trackBeatsUpdated(mixxx::BeatsPointer pBeats) override;

    double getTrackSamples() const;

  signals:
    void loopReset();
    void loopEnabledChanged(bool enabled);
    void loopUpdated(mixxx::audio::FramePos startPosition, mixxx::audio::FramePos endPosition);

  public slots:
    void slotLoopIn(double pressed);
    void slotLoopInGoto(double);
    void slotLoopOut(double pressed);
    void slotLoopOutGoto(double);
    void slotLoopExit(double);
    void slotReloopToggle(double);
    void slotReloopAndStop(double);
    void slotLoopStartPos(double);
    void slotLoopEndPos(double);

    // Generate a loop of 'beats' length. It can also do fractions for a
    // beatslicing effect.
    void slotBeatLoop(double loopSize, bool keepStartPoint=false, bool enable=true);
    void slotBeatLoopSizeChangeRequest(double beats);
    void slotBeatLoopToggle(double pressed);
    void slotBeatLoopRollActivate(double pressed);
    void slotBeatLoopActivate(BeatLoopingControl* pBeatLoopControl);
    void slotBeatLoopActivateRoll(BeatLoopingControl* pBeatLoopControl);
    void slotBeatLoopDeactivate(BeatLoopingControl* pBeatLoopControl);
    void slotBeatLoopDeactivateRoll(BeatLoopingControl* pBeatLoopControl);

    // Jump forward or backward by beats.
    void slotBeatJump(double beats);
    void slotBeatJumpSizeChangeRequest(double beats);
    void slotBeatJumpSizeHalve(double pressed);
    void slotBeatJumpSizeDouble(double pressed);
    void slotBeatJumpForward(double pressed);
    void slotBeatJumpBackward(double pressed);

    // Move the loop by beats.
    void slotLoopMove(double beats);

    void slotLoopScale(double scaleFactor);
    void slotLoopDouble(double pressed);
    void slotLoopHalve(double pressed);
    void slotLoopRemove();

  private slots:
    void slotLoopEnabledValueChangeRequest(double enabled);

  private:
    enum class LoopSeekMode {
        Changed, // force the playposition to be inside the loop after adjusting it.
        MovedOut,
        None,
    };

    struct LoopInfo {
        mixxx::audio::FramePos startPosition;
        mixxx::audio::FramePos endPosition;
        LoopSeekMode seekMode;
    };

    void setLoopingEnabled(bool enabled);
    void setLoopInToCurrentPosition();
    void setLoopOutToCurrentPosition();
    void clearActiveBeatLoop();
    void updateBeatLoopingControls();
    bool currentLoopMatchesBeatloopSize(const LoopInfo& loopInfo) const;

    // Given loop in and out points, determine if this is a beatloop of a particular
    // size.
    double findBeatloopSizeForLoop(mixxx::audio::FramePos startPosition,
            mixxx::audio::FramePos endPosition) const;
    // When a loop changes size such that the playposition is outside of the loop,
    // we can figure out the best place in the new loop to seek to maintain
    // the beat. It will even keep multi-bar phrasing correct with 4/4 tracks.
    mixxx::audio::FramePos adjustedPositionInsideAdjustedLoop(
            mixxx::audio::FramePos currentPosition,
            bool reverse,
            mixxx::audio::FramePos oldLoopInPosition,
            mixxx::audio::FramePos oldLoopOutPosition,
            mixxx::audio::FramePos newLoopInPosition,
            mixxx::audio::FramePos newLoopOutPosition);
    mixxx::audio::FramePos findQuantizedBeatloopStart(
            const mixxx::BeatsPointer& pBeats,
            mixxx::audio::FramePos currentPosition,
            double beats) const;

    ControlPushButton* m_pCOBeatLoopActivate;
    ControlPushButton* m_pCOBeatLoopRollActivate;
    ControlObject* m_pCOLoopStartPosition;
    ControlObject* m_pCOLoopEndPosition;
    ControlObject* m_pCOLoopEnabled;
    ControlPushButton* m_pLoopInButton;
    ControlPushButton* m_pLoopInGotoButton;
    ControlPushButton* m_pLoopOutButton;
    ControlPushButton* m_pLoopOutGotoButton;
    ControlPushButton* m_pLoopExitButton;
    ControlPushButton* m_pReloopToggleButton;
    ControlPushButton* m_pReloopAndStopButton;
    ControlObject* m_pCOLoopScale;
    ControlPushButton* m_pLoopHalveButton;
    ControlPushButton* m_pLoopDoubleButton;
    ControlPushButton* m_pLoopRemoveButton;
    ControlObject* m_pSlipEnabled;
    RateControl* m_pRateControl;
    ControlObject* m_pPlayButton;
    ControlObject* m_pRepeatButton;

    bool m_bLoopingEnabled;
    bool m_bLoopRollActive;
    bool m_bLoopWasEnabledBeforeSlipEnable;
    bool m_bAdjustingLoopIn;
    bool m_bAdjustingLoopOut;
    bool m_bAdjustingLoopInOld;
    bool m_bAdjustingLoopOutOld;
    bool m_bLoopOutPressedWhileLoopDisabled;
    QStack<double> m_activeLoopRolls;
    ControlValueAtomic<LoopInfo> m_loopInfo;
    LoopInfo m_oldLoopInfo;
    ControlValueAtomic<mixxx::audio::FramePos> m_currentPosition;
    ControlObject* m_pQuantizeEnabled;
    QAtomicPointer<BeatLoopingControl> m_pActiveBeatLoop;

    // Base BeatLoop Control Object.
    ControlObject* m_pCOBeatLoop;
    ControlObject* m_pCOBeatLoopSize;
    // Different sizes for Beat Loops/Seeks.
    static double s_dBeatSizes[];
    // Array of BeatLoopingControls, one for each size.
    QList<BeatLoopingControl*> m_beatLoops;

    ControlObject* m_pCOBeatJump;
    ControlObject* m_pCOBeatJumpSize;
    ControlPushButton* m_pCOBeatJumpSizeHalve;
    ControlPushButton* m_pCOBeatJumpSizeDouble;
    ControlPushButton* m_pCOBeatJumpForward;
    ControlPushButton* m_pCOBeatJumpBackward;
    QList<BeatJumpControl*> m_beatJumps;

    ControlObject* m_pCOLoopMove;
    QList<LoopMoveControl*> m_loopMoves;

    // objects below are written from an engine worker thread
    TrackPointer m_pTrack;
    mixxx::BeatsPointer m_pBeats;

    friend class LoopingControlTest;
};

// Class for handling loop moves of a set size. This allows easy access from
// skins.
class LoopMoveControl : public QObject {
    Q_OBJECT
  public:
    LoopMoveControl(const QString& group, double size);
    virtual ~LoopMoveControl();

  signals:
    void loopMove(double beats);

  public slots:
    void slotMoveForward(double value);
    void slotMoveBackward(double value);

  private:
    double m_dLoopMoveSize;
    ControlPushButton* m_pMoveForward;
    ControlPushButton* m_pMoveBackward;
};

// Class for handling beat jumps of a set size. This allows easy access from
// skins.
class BeatJumpControl : public QObject {
    Q_OBJECT
  public:
    BeatJumpControl(const QString& group, double size);
    virtual ~BeatJumpControl();

  signals:
    void beatJump(double beats);

  public slots:
    void slotJumpForward(double pressed);
    void slotJumpBackward(double pressed);

  private:
    double m_dBeatJumpSize;
    ControlPushButton* m_pJumpForward;
    ControlPushButton* m_pJumpBackward;
};

// Class for handling beat loops of a set size. This allows easy access from
// skins.
class BeatLoopingControl : public QObject {
    Q_OBJECT
  public:
    BeatLoopingControl(const QString& group, double size);
    virtual ~BeatLoopingControl();

    void activate();
    void deactivate();
    inline double getSize() {
        return m_dBeatLoopSize;
    }
  public slots:
    void slotLegacy(double value);
    void slotActivate(double value);
    void slotActivateRoll(double value);
    void slotToggle(double value);

  signals:
    void activateBeatLoop(BeatLoopingControl*);
    void deactivateBeatLoop(BeatLoopingControl*);
    void activateBeatLoopRoll(BeatLoopingControl*);
    void deactivateBeatLoopRoll(BeatLoopingControl*);

  private:
    double m_dBeatLoopSize;
    bool m_bActive;
    ControlPushButton* m_pLegacy;
    ControlPushButton* m_pActivate;
    ControlPushButton* m_pActivateRoll;
    ControlPushButton* m_pToggle;
    ControlObject* m_pEnabled;
};
