// loopingcontrol.h
// Created on Sep 23, 2008
// Author: asantoni, rryan

#ifndef LOOPINGCONTROL_H
#define LOOPINGCONTROL_H

#include <QObject>

#include "preferences/usersettings.h"
#include "engine/enginecontrol.h"
#include "track/track.h"
#include "track/beats.h"
#include "control/controlvalue.h"

#define MINIMUM_AUDIBLE_LOOP_SIZE   300  // In samples

class ControlPushButton;
class ControlObject;

class LoopMoveControl;
class BeatJumpControl;
class BeatLoopingControl;

class LoopingControl : public EngineControl {
    Q_OBJECT
  public:
    static QList<double> getBeatSizes();

    LoopingControl(QString group, UserSettingsPointer pConfig);
    virtual ~LoopingControl();

    // process() updates the internal state of the LoopingControl to reflect the
    // correct current sample. If a loop should be taken LoopingControl returns
    // the sample that should be seeked to. Otherwise it returns currentSample.
    virtual double process(const double dRate,
                   const double currentSample,
                   const double totalSamples,
                   const int iBufferSize);

    // nextTrigger returns the sample at which the engine will be triggered to
    // take a loop, given the value of currentSample and dRate.
    virtual double nextTrigger(const double dRate,
                       const double currentSample,
                       const double totalSamples,
                       const int iBufferSize);

    // getTrigger returns the sample that the engine will next be triggered to
    // loop to, given the value of currentSample and dRate.
    virtual double getTrigger(const double dRate,
                      const double currentSample,
                      const double totalSamples,
                      const int iBufferSize);

    // hintReader will add to hintList hints both the loop in and loop out
    // sample, if set.
    virtual void hintReader(HintVector* pHintList);

    virtual void notifySeek(double dNewPlaypos);

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
    virtual void trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) override;
    void slotUpdatedTrackBeats();

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
    void slotBeatJumpForward(double pressed);
    void slotBeatJumpBackward(double pressed);

    // Move the loop by beats.
    void slotLoopMove(double beats);

    void slotLoopScale(double scaleFactor);
    void slotLoopDouble(double pressed);
    void slotLoopHalve(double pressed);

  private:

    struct LoopSamples {
        int start;
        int end;
    };

    void setLoopingEnabled(bool enabled);
    void setLoopInToCurrentPosition();
    void setLoopOutToCurrentPosition();
    void clearActiveBeatLoop();
    void updateBeatLoopingControls();
    int calculateEndOfBeatloop(int startSample, double beatloopSizeInBeats);
    bool currentLoopMatchesBeatloopSize();
    // When a loop changes size such that the playposition is outside of the loop,
    // we can figure out the best place in the new loop to seek to maintain
    // the beat.  It will even keep multi-bar phrasing correct with 4/4 tracks.
    void seekInsideAdjustedLoop(int old_loop_in, int old_loop_out,
                                int new_loop_in, int new_loop_out);

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
    ControlObject* m_pSlipEnabled;
    ControlObject* m_pPlayButton;

    bool m_bLoopingEnabled = false;
    bool m_bLoopRollActive = false;
    bool m_bLoopManualTogglePressedToExitLoop = false;
    bool m_bReloopCatchUpcomingLoop = false;
    bool m_bAdjustingLoopIn = false;
    bool m_bAdjustingLoopOut = false;
    bool m_bLoopOutPressedWhileLoopDisabled = false;
    // TODO(DSC) Make the following values double
    ControlValueAtomic<LoopSamples> m_loopSamples;
    QAtomicInt m_iCurrentSample;
    ControlObject* m_pQuantizeEnabled;
    ControlObject* m_pNextBeat;
    ControlObject* m_pPreviousBeat;
    ControlObject* m_pClosestBeat;
    ControlObject* m_pTrackSamples;
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
    ControlPushButton* m_pCOBeatJumpForward;
    ControlPushButton* m_pCOBeatJumpBackward;
    QList<BeatJumpControl*> m_beatJumps;

    ControlObject* m_pCOLoopMove;
    QList<LoopMoveControl*> m_loopMoves;

    TrackPointer m_pTrack;
    BeatsPointer m_pBeats;
};

// Class for handling loop moves of a set size. This allows easy access from
// skins.
class LoopMoveControl : public QObject {
    Q_OBJECT
  public:
    LoopMoveControl(QString group, double size);
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
    BeatJumpControl(QString group, double size);
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
    BeatLoopingControl(QString group, double size);
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

#endif /* LOOPINGCONTROL_H */
