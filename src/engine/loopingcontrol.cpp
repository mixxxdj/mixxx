// loopingcontrol.cpp
// Created on Sep 23, 2008
// Author: asantoni, rryan

#include <QtDebug>

#include "control/controlobject.h"
#include "preferences/usersettings.h"
#include "control/controlpushbutton.h"
#include "engine/loopingcontrol.h"
#include "engine/bpmcontrol.h"
#include "engine/enginecontrol.h"
#include "util/math.h"
#include "util/sample.h"

#include "track/track.h"
#include "track/beats.h"

double LoopingControl::s_dBeatSizes[] = { 0.03125, 0.0625, 0.125, 0.25, 0.5,
                                          1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };

// Used to generate the beatloop_%SIZE, beatjump_%SIZE, and loop_move_%SIZE CO
// ConfigKeys.
ConfigKey keyForControl(QString group, QString ctrlName, double num) {
    ConfigKey key;
    key.group = group;
    key.item = ctrlName.arg(num);
    return key;
}

// static
QList<double> LoopingControl::getBeatSizes() {
    QList<double> result;
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        result.append(s_dBeatSizes[i]);
    }
    return result;
}

LoopingControl::LoopingControl(QString group,
                               UserSettingsPointer pConfig)
        : EngineControl(group, pConfig) {
    LoopSamples loopSamples = { kNoTrigger, kNoTrigger };
    m_loopSamples.setValue(loopSamples);
    m_iCurrentSample = 0;
    m_pActiveBeatLoop = NULL;

    //Create loop-in, loop-out, loop-exit, and reloop/exit ControlObjects
    m_pLoopInButton = new ControlPushButton(ConfigKey(group, "loop_in"));
    connect(m_pLoopInButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopIn(double)),
            Qt::DirectConnection);
    m_pLoopInButton->set(0);

    m_pLoopInGotoButton = new ControlPushButton(ConfigKey(group, "loop_in_goto"));
    connect(m_pLoopInGotoButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopInGoto(double)));

    m_pLoopOutButton = new ControlPushButton(ConfigKey(group, "loop_out"));
    connect(m_pLoopOutButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopOut(double)),
            Qt::DirectConnection);
    m_pLoopOutButton->set(0);

    m_pLoopOutGotoButton = new ControlPushButton(ConfigKey(group, "loop_out_goto"));
    connect(m_pLoopOutGotoButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopOutGoto(double)));


    m_pLoopExitButton = new ControlPushButton(ConfigKey(group, "loop_exit"));
    connect(m_pLoopExitButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopExit(double)),
            Qt::DirectConnection);
    m_pLoopExitButton->set(0);

    m_pReloopToggleButton = new ControlPushButton(ConfigKey(group, "reloop_toggle"));
    connect(m_pReloopToggleButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotReloopToggle(double)),
            Qt::DirectConnection);
    m_pReloopToggleButton->set(0);
    // The old reloop_exit name was confusing. This CO does both entering and exiting.
    ControlDoublePrivate::insertAlias(ConfigKey(group, "reloop_exit"),
                                      ConfigKey(group, "reloop_toggle"));

    m_pReloopAndStopButton = new ControlPushButton(ConfigKey(group, "reloop_andstop"));
    connect(m_pReloopAndStopButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotReloopAndStop(double)),
            Qt::DirectConnection);

    m_pCOLoopEnabled = new ControlObject(ConfigKey(group, "loop_enabled"));
    m_pCOLoopEnabled->set(0.0);

    m_pCOLoopStartPosition =
            new ControlObject(ConfigKey(group, "loop_start_position"));
    m_pCOLoopStartPosition->set(kNoTrigger);
    connect(m_pCOLoopStartPosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopStartPos(double)),
            Qt::DirectConnection);

    m_pCOLoopEndPosition =
            new ControlObject(ConfigKey(group, "loop_end_position"));
    m_pCOLoopEndPosition->set(kNoTrigger);
    connect(m_pCOLoopEndPosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopEndPos(double)),
            Qt::DirectConnection);

    m_pQuantizeEnabled = ControlObject::getControl(ConfigKey(group, "quantize"));
    m_pNextBeat = ControlObject::getControl(ConfigKey(group, "beat_next"));
    m_pPreviousBeat = ControlObject::getControl(ConfigKey(group, "beat_prev"));
    m_pClosestBeat = ControlObject::getControl(ConfigKey(group, "beat_closest"));
    m_pTrackSamples = ControlObject::getControl(ConfigKey(group, "track_samples"));
    m_pSlipEnabled = ControlObject::getControl(ConfigKey(group, "slip_enabled"));

    // DEPRECATED: Use beatloop_size and beatloop_set instead.
    // Activates a beatloop of a specified number of beats.
    m_pCOBeatLoop = new ControlObject(ConfigKey(group, "beatloop"), false);
    connect(m_pCOBeatLoop, SIGNAL(valueChanged(double)), this,
            SLOT(slotBeatLoop(double)), Qt::DirectConnection);

    m_pCOBeatLoopSize = new ControlObject(ConfigKey(group, "beatloop_size"),
                                          true, false, false, 4.0);
    m_pCOBeatLoopSize->connectValueChangeRequest(this,
            SLOT(slotBeatLoopSizeChangeRequest(double)), Qt::DirectConnection);
    m_pCOBeatLoopActivate = new ControlPushButton(ConfigKey(group, "beatloop_activate"));
    connect(m_pCOBeatLoopActivate, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatLoopToggle(double)));
    m_pCOBeatLoopRollActivate = new ControlPushButton(ConfigKey(group, "beatlooproll_activate"));
    connect(m_pCOBeatLoopRollActivate, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatLoopRollActivate(double)));

    // Here we create corresponding beatloop_(SIZE) CO's which all call the same
    // BeatControl, but with a set value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        BeatLoopingControl* pBeatLoop = new BeatLoopingControl(group, s_dBeatSizes[i]);
        connect(pBeatLoop, SIGNAL(activateBeatLoop(BeatLoopingControl*)),
                this, SLOT(slotBeatLoopActivate(BeatLoopingControl*)),
                Qt::DirectConnection);
        connect(pBeatLoop, SIGNAL(activateBeatLoopRoll(BeatLoopingControl*)),
                this, SLOT(slotBeatLoopActivateRoll(BeatLoopingControl*)),
                Qt::DirectConnection);
        connect(pBeatLoop, SIGNAL(deactivateBeatLoop(BeatLoopingControl*)),
                this, SLOT(slotBeatLoopDeactivate(BeatLoopingControl*)),
                Qt::DirectConnection);
        connect(pBeatLoop, SIGNAL(deactivateBeatLoopRoll(BeatLoopingControl*)),
                this, SLOT(slotBeatLoopDeactivateRoll(BeatLoopingControl*)),
                Qt::DirectConnection);
        m_beatLoops.append(pBeatLoop);
    }

    m_pCOBeatJump = new ControlObject(ConfigKey(group, "beatjump"), false);
    connect(m_pCOBeatJump, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatJump(double)), Qt::DirectConnection);
    m_pCOBeatJumpSize = new ControlObject(ConfigKey(group, "beatjump_size"),
                                          true, false, false, 4.0);
    m_pCOBeatJumpForward = new ControlPushButton(ConfigKey(group, "beatjump_forward"));
    connect(m_pCOBeatJumpForward, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatJumpForward(double)));
    m_pCOBeatJumpBackward = new ControlPushButton(ConfigKey(group, "beatjump_backward"));
    connect(m_pCOBeatJumpBackward, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatJumpBackward(double)));

    // Create beatjump_(SIZE) CO's which all call beatjump, but with a set
    // value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        BeatJumpControl* pBeatJump = new BeatJumpControl(group, s_dBeatSizes[i]);
        connect(pBeatJump, SIGNAL(beatJump(double)),
                this, SLOT(slotBeatJump(double)),
                Qt::DirectConnection);
        m_beatJumps.append(pBeatJump);
    }

    m_pCOLoopMove = new ControlObject(ConfigKey(group, "loop_move"), false);
    connect(m_pCOLoopMove, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopMove(double)), Qt::DirectConnection);

    // Create loop_move_(SIZE) CO's which all call loop_move, but with a set
    // value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        LoopMoveControl* pLoopMove = new LoopMoveControl(group, s_dBeatSizes[i]);
        connect(pLoopMove, SIGNAL(loopMove(double)),
                this, SLOT(slotLoopMove(double)),
                Qt::DirectConnection);
        m_loopMoves.append(pLoopMove);
    }

    m_pCOLoopScale = new ControlObject(ConfigKey(group, "loop_scale"), false);
    connect(m_pCOLoopScale, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopScale(double)));
    m_pLoopHalveButton = new ControlPushButton(ConfigKey(group, "loop_halve"));
    connect(m_pLoopHalveButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopHalve(double)));
    m_pLoopDoubleButton = new ControlPushButton(ConfigKey(group, "loop_double"));
    connect(m_pLoopDoubleButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopDouble(double)));

    m_pPlayButton = ControlObject::getControl(ConfigKey(group, "play"));
}

LoopingControl::~LoopingControl() {
    delete m_pLoopOutButton;
    delete m_pLoopOutGotoButton;
    delete m_pLoopInButton;
    delete m_pLoopInGotoButton;
    delete m_pLoopExitButton;
    delete m_pReloopToggleButton;
    delete m_pReloopAndStopButton;
    delete m_pCOLoopEnabled;
    delete m_pCOLoopStartPosition;
    delete m_pCOLoopEndPosition;
    delete m_pCOLoopScale;
    delete m_pLoopHalveButton;
    delete m_pLoopDoubleButton;

    delete m_pCOBeatLoop;
    while (!m_beatLoops.isEmpty()) {
        BeatLoopingControl* pBeatLoop = m_beatLoops.takeLast();
        delete pBeatLoop;
    }
    delete m_pCOBeatLoopSize;
    delete m_pCOBeatLoopActivate;
    delete m_pCOBeatLoopRollActivate;

    delete m_pCOBeatJump;
    delete m_pCOBeatJumpSize;
    delete m_pCOBeatJumpForward;
    delete m_pCOBeatJumpBackward;
    while (!m_beatJumps.isEmpty()) {
        BeatJumpControl* pBeatJump = m_beatJumps.takeLast();
        delete pBeatJump;
    }

    delete m_pCOLoopMove;
    while (!m_loopMoves.isEmpty()) {
        LoopMoveControl* pLoopMove = m_loopMoves.takeLast();
        delete pLoopMove;
    }
}

void LoopingControl::slotLoopScale(double scaleFactor) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || loopSamples.end == kNoTrigger) {
        return;
    }
    int loop_length = loopSamples.end - loopSamples.start;
    int old_loop_end = loopSamples.end;
    int samples = m_pTrackSamples->get();
    loop_length *= scaleFactor;

    // Abandon loops that are too short of extend beyond the end of the file.
    if (loop_length < MINIMUM_AUDIBLE_LOOP_SIZE ||
            loopSamples.start + loop_length > samples) {
        return;
    }

    loopSamples.end = loopSamples.start + loop_length;

    if (!even(loopSamples.end)) {
        loopSamples.end--;
    }

    // TODO(XXX) we could be smarter about taking the active beatloop, scaling
    // it by the desired amount and trying to find another beatloop that matches
    // it, but for now we just clear the active beat loop if somebody scales.
    clearActiveBeatLoop();

    // Don't allow 0 samples loop, so one can still manipulate it
    if (loopSamples.end == loopSamples.start) {
        if ((loopSamples.end + 2) >= samples)
            loopSamples.start -= 2;
        else
            loopSamples.end += 2;
    }
    // Do not allow loops to go past the end of the song
    else if (loopSamples.end > samples) {
        loopSamples.end = samples;
    }

    m_loopSamples.setValue(loopSamples);

    // Update CO for loop end marker
    m_pCOLoopEndPosition->set(loopSamples.end);

    // Reseek if the loop shrank out from under the playposition.
    if (m_bLoopingEnabled && scaleFactor < 1.0) {
        seekInsideAdjustedLoop(
                loopSamples.start, old_loop_end,
                loopSamples.start, loopSamples.end);
    }
}

void LoopingControl::slotLoopHalve(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    slotBeatLoop(m_pCOBeatLoopSize->get() / 2.0, true, false);
}

void LoopingControl::slotLoopDouble(double pressed) {
    if (pressed <= 0.0) {
        return;
    }

    slotBeatLoop(m_pCOBeatLoopSize->get() * 2.0, true, false);
}

double LoopingControl::process(const double dRate,
                               const double currentSample,
                               const double totalSamples,
                               const int iBufferSize) {
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBufferSize);

    int currentSampleEven = static_cast<int>(currentSample);
    if (!even(currentSampleEven)) {
        currentSampleEven--;
    }
    m_iCurrentSample = currentSampleEven;

    bool reverse = dRate < 0;

    double retval = kNoTrigger;
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (m_bLoopingEnabled &&
            loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {
        bool outsideLoop = currentSample >= loopSamples.end ||
                           currentSample <= loopSamples.start;
        if (outsideLoop) {
            if (!m_bReloopCatchUpcomingLoop && !m_bAdjustingLoopIn && !m_bAdjustingLoopOut) {
                retval = reverse ? loopSamples.end : loopSamples.start;
            }
        } else {
            m_bReloopCatchUpcomingLoop = false;
        }
    }

    if (m_bAdjustingLoopIn) {
        setLoopInToCurrentPosition();
    } else if (m_bAdjustingLoopOut) {
        setLoopOutToCurrentPosition();
    }

    return retval;
}

double LoopingControl::nextTrigger(const double dRate,
                                   const double currentSample,
                                   const double totalSamples,
                                   const int iBufferSize) {
    Q_UNUSED(currentSample);
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBufferSize);
    bool bReverse = dRate < 0;
    LoopSamples loopSamples = m_loopSamples.getValue();

    if (m_bLoopingEnabled && !m_bReloopCatchUpcomingLoop &&
            !m_bAdjustingLoopIn && !m_bAdjustingLoopOut) {
        if (bReverse) {
            return loopSamples.start;
        } else {
            return loopSamples.end;
        }
    }
    return kNoTrigger;
}

double LoopingControl::getTrigger(const double dRate,
                                  const double currentSample,
                                  const double totalSamples,
                                  const int iBufferSize) {
    Q_UNUSED(currentSample);
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBufferSize);
    bool bReverse = dRate < 0;
    LoopSamples loopSamples = m_loopSamples.getValue();

    if (m_bLoopingEnabled && !m_bReloopCatchUpcomingLoop &&
            !m_bAdjustingLoopIn && !m_bAdjustingLoopOut) {
        if (bReverse) {
            return loopSamples.end;
        } else {
            return loopSamples.start;
        }
    }
    return kNoTrigger;
}

void LoopingControl::hintReader(HintVector* pHintList) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    Hint loop_hint;
    // If the loop is enabled, then this is high priority because we will loop
    // sometime potentially very soon! The current audio itself is priority 1,
    // but we will issue ourselves at priority 2.
    if (m_bLoopingEnabled) {
        // If we're looping, hint the loop in and loop out, in case we reverse
        // into it. We could save information from process to tell which
        // direction we're going in, but that this is much simpler, and hints
        // aren't that bad to make anyway.
        if (loopSamples.start >= 0) {
            loop_hint.priority = 2;
            loop_hint.frame = SampleUtil::floorPlayPosToFrame(loopSamples.start);
            loop_hint.frameCount = Hint::kFrameCountForward;
            pHintList->append(loop_hint);
        }
        if (loopSamples.end >= 0) {
            loop_hint.priority = 10;
            loop_hint.frame = SampleUtil::ceilPlayPosToFrame(loopSamples.end);
            loop_hint.frameCount = Hint::kFrameCountBackward;
            pHintList->append(loop_hint);
        }
    } else {
        if (loopSamples.start >= 0) {
            loop_hint.priority = 10;
            loop_hint.frame = SampleUtil::floorPlayPosToFrame(loopSamples.start);
            loop_hint.frameCount = Hint::kFrameCountForward;
            pHintList->append(loop_hint);
        }
    }
}

void LoopingControl::setLoopInToCurrentPosition() {
    // set loop-in position
    LoopSamples loopSamples = m_loopSamples.getValue();
    double quantizedBeat = -1;
    int pos = m_iCurrentSample;
    if (m_pQuantizeEnabled->toBool() && m_pBeats != nullptr) {
        if (m_bAdjustingLoopIn) {
            double closestBeat = m_pClosestBeat->get();
            if (closestBeat == getCurrentSample()) {
                quantizedBeat = closestBeat;
            } else {
                quantizedBeat = m_pPreviousBeat->get();
            }
        } else {
            quantizedBeat = m_pClosestBeat->get();
        }
        if (quantizedBeat != -1) {
            pos = static_cast<int>(floor(quantizedBeat));
        }
    }

    if (pos != -1 && !even(pos)) {
        pos--;
    }

    // Reset the loop out position if it is before the loop in so that loops
    // cannot be inverted.
    if (loopSamples.end != kNoTrigger &&
            loopSamples.end < pos) {
        loopSamples.end = kNoTrigger;
        m_pCOLoopEndPosition->set(kNoTrigger);
    }

    // If we're looping and the loop-in and out points are now so close
    //  that the loop would be inaudible, set the in point to the smallest
    //  pre-defined beatloop size instead (when possible)
    if (loopSamples.end != kNoTrigger &&
            (loopSamples.end - pos) < MINIMUM_AUDIBLE_LOOP_SIZE) {
        if (quantizedBeat != -1 && m_pBeats) {
            pos = static_cast<int>(floor(m_pBeats->findNthBeat(quantizedBeat, -2)));
            if (pos == -1 || (loopSamples.end - pos) < MINIMUM_AUDIBLE_LOOP_SIZE) {
                pos = loopSamples.end - MINIMUM_AUDIBLE_LOOP_SIZE;
            }
        } else {
            pos = loopSamples.end - MINIMUM_AUDIBLE_LOOP_SIZE;
        }
    }

    loopSamples.start = pos;
    m_pCOLoopStartPosition->set(loopSamples.start);

    if (m_pQuantizeEnabled->toBool()
            && loopSamples.start < loopSamples.end
            && m_pBeats != nullptr) {
        m_pCOBeatLoopSize->setAndConfirm(
            m_pBeats->numBeatsInRange(loopSamples.start, loopSamples.end));
        updateBeatLoopingControls();
    } else {
        clearActiveBeatLoop();
    }

    m_loopSamples.setValue(loopSamples);
    //qDebug() << "set loop_in to " << loopSamples.start;
}

void LoopingControl::slotLoopIn(double pressed) {
    if (m_pTrack == nullptr) {
        return;
    }

    // If loop is enabled, suspend looping and set the loop in point
    // when this button is released.
    if (m_bLoopingEnabled) {
        if (pressed > 0.0) {
            m_bAdjustingLoopIn = true;
            // Adjusting both the in and out point at the same time makes no sense
            m_bAdjustingLoopOut = false;
        } else {
            setLoopInToCurrentPosition();
            m_bAdjustingLoopIn = false;
        }
    } else {
        if (pressed > 0.0) {
            setLoopInToCurrentPosition();
        }
        m_bAdjustingLoopIn = false;
    }
}

void LoopingControl::slotLoopInGoto(double pressed) {
    if (pressed > 0.0) {
        seekAbs(static_cast<double>(
            m_loopSamples.getValue().start));
    }
}

void LoopingControl::setLoopOutToCurrentPosition() {
    LoopSamples loopSamples = m_loopSamples.getValue();
    double quantizedBeat = -1;
    int pos = m_iCurrentSample;
    if (m_pQuantizeEnabled->toBool() && m_pBeats != nullptr) {
        if (m_bAdjustingLoopOut) {
            double closestBeat = m_pClosestBeat->get();
            if (closestBeat == getCurrentSample()) {
                quantizedBeat = closestBeat;
            } else {
                quantizedBeat = m_pNextBeat->get();
            }
        } else {
            quantizedBeat = m_pClosestBeat->get();
        }
        if (quantizedBeat != -1) {
            pos = static_cast<int>(floor(quantizedBeat));
        }
    }

    if (pos != -1 && !even(pos)) {
        pos++;  // Increment to avoid shortening too-short loops
    }

    // If the user is trying to set a loop-out before the loop in or without
    // having a loop-in, then ignore it.
    if (loopSamples.start == kNoTrigger || pos < loopSamples.start) {
        return;
    }

    // If the loop-in and out points are set so close that the loop would be
    //  inaudible (which can happen easily with quantize-to-beat enabled,)
    //  use the smallest pre-defined beatloop instead (when possible)
    if ((pos - loopSamples.start) < MINIMUM_AUDIBLE_LOOP_SIZE) {
        if (quantizedBeat != -1 && m_pBeats) {
            pos = static_cast<int>(floor(m_pBeats->findNthBeat(quantizedBeat, 2)));
            if (pos == -1 || (pos - loopSamples.start) < MINIMUM_AUDIBLE_LOOP_SIZE) {
                pos = loopSamples.start + MINIMUM_AUDIBLE_LOOP_SIZE;
            }
        } else {
            pos = loopSamples.start + MINIMUM_AUDIBLE_LOOP_SIZE;
        }
    }

    // set loop out position
    loopSamples.end = pos;
    m_pCOLoopEndPosition->set(loopSamples.end);
    m_loopSamples.setValue(loopSamples);

    // start looping
    if (loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {
        setLoopingEnabled(true);
    }

    if (m_pQuantizeEnabled->toBool() && m_pBeats != nullptr) {
        m_pCOBeatLoopSize->setAndConfirm(
            m_pBeats->numBeatsInRange(loopSamples.start, loopSamples.end));
        updateBeatLoopingControls();
    } else {
        clearActiveBeatLoop();
    }
    //qDebug() << "set loop_out to " << loopSamples.end;
}

void LoopingControl::slotLoopOut(double pressed) {
    if (m_pTrack == nullptr) {
        return;
    }

    // If loop is enabled, suspend looping and set the loop out point
    // when this button is released.
    if (m_bLoopingEnabled) {
        if (pressed > 0.0) {
            m_bAdjustingLoopOut = true;
            // Adjusting both the in and out point at the same time makes no sense
            m_bAdjustingLoopIn = false;
        } else {
            // If this button was pressed to set the loop out point when loop
            // was disabled, that will enable looping, so avoid moving the
            // loop out point when the button is released.
            if (!m_bLoopOutPressedWhileLoopDisabled) {
                setLoopOutToCurrentPosition();
                m_bAdjustingLoopOut = false;
            } else {
                m_bLoopOutPressedWhileLoopDisabled = false;
            }
        }
    } else {
        if (pressed > 0.0) {
            setLoopOutToCurrentPosition();
            m_bLoopOutPressedWhileLoopDisabled = true;
        }
        m_bAdjustingLoopOut = false;
    }
}

void LoopingControl::slotLoopOutGoto(double pressed) {
    if (pressed > 0.0) {
        seekAbs(static_cast<double>(
            m_loopSamples.getValue().end));
    }
}

void LoopingControl::slotLoopExit(double val) {
    if (!m_pTrack || val <= 0.0) {
        return;
    }

    // If we're looping, stop looping
    if (m_bLoopingEnabled) {
        setLoopingEnabled(false);
    }
}

void LoopingControl::slotReloopToggle(double val) {
    if (!m_pTrack || val <= 0.0) {
        return;
    }

    // If we're looping, stop looping
    if (m_bLoopingEnabled) {
        // If loop roll was active, also disable slip.
        if (m_bLoopRollActive) {
            m_pSlipEnabled->set(0);
            m_bLoopRollActive = false;
        }
        setLoopingEnabled(false);
        //qDebug() << "reloop_toggle looping off";
    } else {
        // If we're not looping, enable the loop. If the loop is ahead of the
        // current play position, do not jump to it.
        LoopSamples loopSamples = m_loopSamples.getValue();
        if (loopSamples.start != kNoTrigger && loopSamples.end != kNoTrigger &&
                loopSamples.start <= loopSamples.end) {
            if (getCurrentSample() < loopSamples.start) {
                m_bReloopCatchUpcomingLoop = true;
            }
            setLoopingEnabled(true);
            // If we're not playing, jump to the loop in point so the waveform
            // shows where it will play from when playback resumes.
            if (!m_pPlayButton->toBool() && !m_bReloopCatchUpcomingLoop) {
                slotLoopInGoto(1);
            }
        }
        //qDebug() << "reloop_toggle looping on";
    }
}

void LoopingControl::slotReloopAndStop(double pressed) {
    if (pressed > 0) {
        m_pPlayButton->set(0.0);
        seekAbs(static_cast<double>(
            m_loopSamples.getValue().start));
        setLoopingEnabled(true);
    }
}

void LoopingControl::slotLoopStartPos(double pos) {
    if (!m_pTrack) {
        return;
    }

    int newpos = pos;
    if (newpos != kNoTrigger && !even(newpos)) {
        newpos--;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();

    if (loopSamples.start == newpos) {
        //nothing to do
        return;
    }

    clearActiveBeatLoop();

    if (pos == kNoTrigger) {
        setLoopingEnabled(false);
    }

    loopSamples.start = newpos;
    m_pCOLoopStartPosition->set(newpos);

    if (loopSamples.end != kNoTrigger &&
            loopSamples.end <= loopSamples.start) {
        loopSamples.end = kNoTrigger;
        m_pCOLoopEndPosition->set(kNoTrigger);
        setLoopingEnabled(false);
    }
    m_loopSamples.setValue(loopSamples);
}

void LoopingControl::slotLoopEndPos(double pos) {
    if (!m_pTrack) {
        return;
    }

    int newpos = pos;
    if (newpos != -1 && !even(newpos)) {
        newpos--;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.end == newpos) {
        //nothing to do
        return;
    }

    // Reject if the loop-in is not set, or if the new position is before the
    // start point (but not -1).
    if (loopSamples.start == kNoTrigger ||
            (newpos != kNoTrigger && newpos <= loopSamples.start)) {
        m_pCOLoopEndPosition->set(loopSamples.end);
        return;
    }

    clearActiveBeatLoop();

    if (pos == -1.0) {
        setLoopingEnabled(false);
    }
    loopSamples.end = newpos;
    m_pCOLoopEndPosition->set(newpos);
    m_loopSamples.setValue(loopSamples);
}

void LoopingControl::notifySeek(double dNewPlaypos) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (m_bLoopingEnabled) {
        // Disable loop when we jump after it, using hot cues or waveform overview
        // If we jump before, the loop it is kept enabled as catching loop
        if (dNewPlaypos > loopSamples.end) {
            setLoopingEnabled(false);
        }
    }
}

void LoopingControl::setLoopingEnabled(bool enabled) {
    m_bLoopingEnabled = enabled;
    m_pCOLoopEnabled->set(enabled);
    BeatLoopingControl* pActiveBeatLoop = m_pActiveBeatLoop;
    if (pActiveBeatLoop != nullptr) {
        if (enabled) {
            pActiveBeatLoop->activate();
        } else {
            pActiveBeatLoop->deactivate();
        }
    }
}

void LoopingControl::trackLoaded(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pOldTrack);

    if (m_pTrack) {
        disconnect(m_pTrack.get(), SIGNAL(beatsUpdated()),
                   this, SLOT(slotUpdatedTrackBeats()));
    }

    clearActiveBeatLoop();

    if (pNewTrack) {
        m_pTrack = pNewTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.get(), SIGNAL(beatsUpdated()),
                this, SLOT(slotUpdatedTrackBeats()));
    } else {
        m_pTrack.reset();
        m_pBeats.clear();
    }
}

void LoopingControl::slotUpdatedTrackBeats()
{
    if (m_pTrack) {
        m_pBeats = m_pTrack->getBeats();
    }
}

void LoopingControl::slotBeatLoopActivate(BeatLoopingControl* pBeatLoopControl) {
    if (!m_pTrack) {
        return;
    }

    // Maintain the current start point if there is an active loop currently
    // looping. slotBeatLoop will update m_pActiveBeatLoop if applicable. Note,
    // this used to only maintain the current start point if a beatloop was
    // enabled. See Bug #1159243.
    slotBeatLoop(pBeatLoopControl->getSize(), m_bLoopingEnabled, true);
}

void LoopingControl::slotBeatLoopActivateRoll(BeatLoopingControl* pBeatLoopControl) {
     if (!m_pTrack) {
         return;
     }

    // Disregard existing loops.
    m_pSlipEnabled->set(1);
    slotBeatLoop(pBeatLoopControl->getSize(), false, true);
    m_bLoopRollActive = true;
}

void LoopingControl::slotBeatLoopDeactivate(BeatLoopingControl* pBeatLoopControl) {
    Q_UNUSED(pBeatLoopControl);
    setLoopingEnabled(false);
}

void LoopingControl::slotBeatLoopDeactivateRoll(BeatLoopingControl* pBeatLoopControl) {
    Q_UNUSED(pBeatLoopControl);
    setLoopingEnabled(false);
    // Make sure slip mode is not turned off if it was turned on
    // by something that was not a rolling beatloop.
    if (m_bLoopRollActive) {
        m_pSlipEnabled->set(0);
        m_bLoopRollActive = false;
    }
}

void LoopingControl::clearActiveBeatLoop() {
    BeatLoopingControl* pOldBeatLoop = m_pActiveBeatLoop.fetchAndStoreAcquire(nullptr);
    if (pOldBeatLoop != nullptr) {
        pOldBeatLoop->deactivate();
    }
}

bool LoopingControl::currentLoopMatchesBeatloopSize() {
    if (m_pBeats == nullptr) {
        return false;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();

    // Calculate where the loop out point would be if it is a beatloop
    int beatLoopOutPoint =
        m_pBeats->findNBeatsFromSample(loopSamples.start, m_pCOBeatLoopSize->get());
    if (!even(beatLoopOutPoint)) {
        beatLoopOutPoint--;
    }

    return loopSamples.end == beatLoopOutPoint;
}

void LoopingControl::updateBeatLoopingControls() {
    // O(n) search, but there are only ~10-ish beatloop controls so this is
    // fine.
    double dBeatloopSize = m_pCOBeatLoopSize->get();
    for (BeatLoopingControl* pBeatLoopControl: m_beatLoops) {
        if (pBeatLoopControl->getSize() == dBeatloopSize) {
            if (m_bLoopingEnabled) {
                pBeatLoopControl->activate();
            }
            BeatLoopingControl* pOldBeatLoop =
                    m_pActiveBeatLoop.fetchAndStoreRelease(pBeatLoopControl);
            if (pOldBeatLoop != nullptr && pOldBeatLoop != pBeatLoopControl) {
                pOldBeatLoop->deactivate();
            }
            return;
        }
    }
    // If the loop did not return from the function yet, dBeatloopSize does
    // not match any of the BeatLoopingControls' sizes.
    clearActiveBeatLoop();
}

void LoopingControl::slotBeatLoop(double beats, bool keepStartPoint, bool enable) {
    double maxBeatSize = s_dBeatSizes[sizeof(s_dBeatSizes)/sizeof(s_dBeatSizes[0]) - 1];
    double minBeatSize = s_dBeatSizes[0];
    if (beats < 0) {
        // For now we do not handle negative beatloops.
        clearActiveBeatLoop();
        return;
    } else if (beats > maxBeatSize) {
        beats = maxBeatSize;
    } else if (beats < minBeatSize) {
        beats = minBeatSize;
    }

    int samples = m_pTrackSamples->get();
    if (!m_pTrack || samples == 0
            || !m_pBeats) {
        clearActiveBeatLoop();
        m_pCOBeatLoopSize->setAndConfirm(beats);
        return;
    }

    // Calculate the new loop start and end samples
    // give start and end defaults so we can detect problems
    LoopSamples newloopSamples = {kNoTrigger, kNoTrigger};
    LoopSamples loopSamples = m_loopSamples.getValue();

    // Start from the current position/closest beat and
    // create the loop around X beats from there.
    if (keepStartPoint) {
        if (loopSamples.start != kNoTrigger) {
            newloopSamples.start = loopSamples.start;
        } else {
            newloopSamples.start = getCurrentSample();
        }
    } else {
        // loop_in is set to the previous beat if quantize is on.  The
        // closest beat might be ahead of play position which would cause a seek.
        // TODO: If in reverse, should probably choose nextBeat.
        double cur_pos = getCurrentSample();
        double prevBeat;
        double nextBeat;
        m_pBeats->findPrevNextBeats(cur_pos, &prevBeat, &nextBeat);

        if (m_pQuantizeEnabled->toBool() && prevBeat != -1) {
            if (beats >= 1.0) {
                newloopSamples.start = prevBeat;
            } else {
                // In case of beat length less then 1 beat:
                // (| - beats, ^ - current track's position):
                //
                // ...|...................^........|...
                //
                // If we press 1/2 beatloop we want loop from 50% to 100%,
                // If I press 1/4 beatloop, we want loop from 50% to 75% etc
                double beat_len = nextBeat - prevBeat;
                double loops_per_beat = 1.0 / beats;
                double beat_pos = cur_pos - prevBeat;
                int beat_frac =
                        static_cast<int>(floor((beat_pos / beat_len) *
                                                loops_per_beat));
                newloopSamples.start = prevBeat + beat_len / loops_per_beat * beat_frac;
            }

        } else {
            newloopSamples.start = floor(cur_pos);
        }
    }

    if (!even(newloopSamples.start)) {
        newloopSamples.start--;
    }

    newloopSamples.end = m_pBeats->findNBeatsFromSample(newloopSamples.start, beats);
    if (!even(newloopSamples.end)) {
        newloopSamples.end--;
    }

    if (newloopSamples.start == newloopSamples.end) {
        if ((newloopSamples.end + 2) > samples) {
            newloopSamples.start -= 2;
        } else {
            newloopSamples.end += 2;
        }
    }

    // Do not allow beat loops to go beyond the end of the track
    if (newloopSamples.end > samples) {
        // If a track is loaded with beatloop_size larger than
        // the distance between the loop in point and
        // the end of the track, let beatloop_size be set to
        // a smaller size, but not get larger.
        double previousBeatloopSize = m_pCOBeatLoopSize->get();
        double previousBeatloopOutPoint =
            m_pBeats->findNBeatsFromSample(newloopSamples.start, previousBeatloopSize);
        if (previousBeatloopOutPoint < newloopSamples.start
                && beats < previousBeatloopSize) {
            m_pCOBeatLoopSize->setAndConfirm(beats);
        }
        return;
    }

    // When loading a new track or after setting a manual loop without quantize,
    // do not resize the existing loop until beatloop_size matches
    // the size of the existing loop.
    // Do not return immediately so beatloop_size can be updated.
    bool avoidResize = false;
    if (!currentLoopMatchesBeatloopSize() && !enable) {
        avoidResize = true;
    }

    if (m_pCOBeatLoopSize->get() != beats) {
        m_pCOBeatLoopSize->setAndConfirm(beats);
    }

    // This check happens after setting m_pCOBeatLoopSize so
    // beatloop_size can be prepared without having a track loaded.
    if ((newloopSamples.start == kNoTrigger) || (newloopSamples.end == kNoTrigger)) {
        return;
    }

    if (avoidResize) {
        return;
    }

    // If resizing an inactive loop by changing beatloop_size,
    // do not seek to the adjusted loop.
    if (keepStartPoint && (enable || m_bLoopingEnabled)) {
        seekInsideAdjustedLoop(loopSamples.start, loopSamples.end,
                newloopSamples.start, newloopSamples.end);
    }

    m_loopSamples.setValue(newloopSamples);
    m_pCOLoopStartPosition->set(newloopSamples.start);
    m_pCOLoopEndPosition->set(newloopSamples.end);
    if (enable) {
        setLoopingEnabled(true);
    }
    updateBeatLoopingControls();
}

void LoopingControl::slotBeatLoopSizeChangeRequest(double beats) {
    // slotBeatLoop will call m_pCOBeatLoopSize->setAndConfirm if
    // new beatloop_size is valid
    slotBeatLoop(beats, true, false);
}

void LoopingControl::slotBeatLoopToggle(double pressed) {
    if (pressed > 0) {
        slotBeatLoop(m_pCOBeatLoopSize->get());
    }
}

void LoopingControl::slotBeatLoopRollActivate(double pressed) {
    if (pressed > 0.0) {
        if (m_bLoopingEnabled) {
            setLoopingEnabled(false);
            // Make sure slip mode is not turned off if it was turned on
            // by something that was not a rolling beatloop.
            if (m_bLoopRollActive) {
                m_pSlipEnabled->set(0.0);
                m_bLoopRollActive = false;
            }
        } else {
            m_pSlipEnabled->set(1.0);
            slotBeatLoop(m_pCOBeatLoopSize->get());
            m_bLoopRollActive = true;
        }
    } else {
        setLoopingEnabled(false);
        // Make sure slip mode is not turned off if it was turned on
        // by something that was not a rolling beatloop.
        if (m_bLoopRollActive) {
            m_pSlipEnabled->set(0.0);
            m_bLoopRollActive = false;
        }
    }
}

void LoopingControl::slotBeatJump(double beats) {
    if (!m_pTrack || !m_pBeats) {
        return;
    }

    if (m_bLoopingEnabled && !m_bAdjustingLoopIn && !m_bAdjustingLoopOut) {
        slotLoopMove(beats);
    } else {
        seekAbs(m_pBeats->findNBeatsFromSample(getCurrentSample(), beats));
    }
}

void LoopingControl::slotBeatJumpForward(double pressed) {
    if (pressed) {
        slotBeatJump(m_pCOBeatJumpSize->get());
    }
}

void LoopingControl::slotBeatJumpBackward(double pressed) {
    if (pressed) {
        slotBeatJump(-1.0 * m_pCOBeatJumpSize->get());
    }
}

void LoopingControl::slotLoopMove(double beats) {
    if (m_pTrack == nullptr || m_pBeats == nullptr || beats == 0) {
        return;
    }
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || loopSamples.end == kNoTrigger) {
        return;
    }

    if (BpmControl::getBeatContext(m_pBeats, getCurrentSample(),
                                   nullptr, nullptr, nullptr, nullptr)) {
        int old_loop_in = loopSamples.start;
        int old_loop_out = loopSamples.end;
        int new_loop_in = m_pBeats->findNBeatsFromSample(old_loop_in, beats);
        int new_loop_out = m_pBeats->findNBeatsFromSample(old_loop_out, beats);
        if (!even(new_loop_in)) {
            --new_loop_in;
        }
        if (!even(new_loop_out)) {
            --new_loop_out;
        }

        loopSamples.start = new_loop_in;
        m_pCOLoopStartPosition->set(new_loop_in);
        loopSamples.end = new_loop_out;
        m_pCOLoopEndPosition->set(new_loop_out);
        m_loopSamples.setValue(loopSamples);

        // If we are looping make sure that the play head does not leave the
        // loop as a result of our adjustment.
        if (m_bLoopingEnabled) {
            seekInsideAdjustedLoop(old_loop_in, old_loop_out,
                                   new_loop_in, new_loop_out);
        }
    }
}

void LoopingControl::seekInsideAdjustedLoop(int old_loop_in, int old_loop_out,
                                            int new_loop_in, int new_loop_out) {
    // Copy on stack since m_iCurrentSample sample can change under us.
    int currentSample = m_iCurrentSample;
    if (currentSample >= new_loop_in && currentSample <= new_loop_out) {
        return;
    }

    int new_loop_size = new_loop_out - new_loop_in;
    if (!even(new_loop_size)) {
        --new_loop_size;
    }
    if (new_loop_size > old_loop_out - old_loop_in) {
        // Could this happen if the user grows a loop and then also shifts it?
        qWarning() << "seekInsideAdjustedLoop called for loop that got larger -- ignoring";
        return;
    }

    int adjusted_position = currentSample;
    while (adjusted_position > new_loop_out) {
        adjusted_position -= new_loop_size;
        VERIFY_OR_DEBUG_ASSERT(adjusted_position > new_loop_in) {
            // I'm not even sure this is possible.  The new loop would have to be bigger than the
            // old loop, and the playhead was somehow outside the old loop.
            qWarning() << "SHOULDN'T HAPPEN: seekInsideAdjustedLoop couldn't find a new position --"
                       << " seeking to in point";
            adjusted_position = new_loop_in;
            break;
        }
    }
    while (adjusted_position < new_loop_in) {
        adjusted_position += new_loop_size;
        VERIFY_OR_DEBUG_ASSERT(adjusted_position < new_loop_out) {
            qWarning() << "SHOULDN'T HAPPEN: seekInsideAdjustedLoop couldn't find a new position --"
                       << " seeking to in point";
            adjusted_position = new_loop_in;
            break;
        }
    }
    if (adjusted_position != currentSample) {
        m_iCurrentSample = adjusted_position;
        seekAbs(static_cast<double>(adjusted_position));
    }
}

BeatJumpControl::BeatJumpControl(QString group, double size)
        : m_dBeatJumpSize(size) {
    m_pJumpForward = new ControlPushButton(
            keyForControl(group, "beatjump_%1_forward", size));
    connect(m_pJumpForward, SIGNAL(valueChanged(double)),
            this, SLOT(slotJumpForward(double)),
            Qt::DirectConnection);
    m_pJumpBackward = new ControlPushButton(
            keyForControl(group, "beatjump_%1_backward", size));
    connect(m_pJumpBackward, SIGNAL(valueChanged(double)),
            this, SLOT(slotJumpBackward(double)),
            Qt::DirectConnection);
}

BeatJumpControl::~BeatJumpControl() {
    delete m_pJumpForward;
    delete m_pJumpBackward;
}

void BeatJumpControl::slotJumpBackward(double pressed) {
    if (pressed > 0) {
        emit(beatJump(-m_dBeatJumpSize));
    }
}

void BeatJumpControl::slotJumpForward(double pressed) {
    if (pressed > 0) {
        emit(beatJump(m_dBeatJumpSize));
    }
}

LoopMoveControl::LoopMoveControl(QString group, double size)
        : m_dLoopMoveSize(size) {
    m_pMoveForward = new ControlPushButton(
            keyForControl(group, "loop_move_%1_forward", size));
    connect(m_pMoveForward, SIGNAL(valueChanged(double)),
            this, SLOT(slotMoveForward(double)),
            Qt::DirectConnection);
    m_pMoveBackward = new ControlPushButton(
            keyForControl(group, "loop_move_%1_backward", size));
    connect(m_pMoveBackward, SIGNAL(valueChanged(double)),
            this, SLOT(slotMoveBackward(double)),
            Qt::DirectConnection);
}

LoopMoveControl::~LoopMoveControl() {
    delete m_pMoveForward;
    delete m_pMoveBackward;
}

void LoopMoveControl::slotMoveBackward(double v) {
    if (v > 0) {
        emit(loopMove(-m_dLoopMoveSize));
    }
}

void LoopMoveControl::slotMoveForward(double v) {
    if (v > 0) {
        emit(loopMove(m_dLoopMoveSize));
    }
}

BeatLoopingControl::BeatLoopingControl(QString group, double size)
        : m_dBeatLoopSize(size),
          m_bActive(false) {
    // This is the original beatloop control which is now deprecated. Its value
    // is the state of the beatloop control (1 for enabled, 0 for disabled).
    m_pLegacy = new ControlPushButton(
            keyForControl(group, "beatloop_%1", size));
    m_pLegacy->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pLegacy, SIGNAL(valueChanged(double)),
            this, SLOT(slotLegacy(double)),
            Qt::DirectConnection);
    // A push-button which activates the beatloop.
    m_pActivate = new ControlPushButton(
            keyForControl(group, "beatloop_%1_activate", size));
    connect(m_pActivate, SIGNAL(valueChanged(double)),
            this, SLOT(slotActivate(double)),
            Qt::DirectConnection);
    // A push-button which toggles the beatloop as active or inactive.
    m_pToggle = new ControlPushButton(
            keyForControl(group, "beatloop_%1_toggle", size));
    connect(m_pToggle, SIGNAL(valueChanged(double)),
            this, SLOT(slotToggle(double)),
            Qt::DirectConnection);

    // A push-button which activates rolling beatloops
    m_pActivateRoll = new ControlPushButton(
            keyForControl(group, "beatlooproll_%1_activate", size));
    connect(m_pActivateRoll, SIGNAL(valueChanged(double)),
            this, SLOT(slotActivateRoll(double)),
            Qt::DirectConnection);

    // An indicator control which is 1 if the beatloop is enabled and 0 if not.
    m_pEnabled = new ControlObject(
            keyForControl(group, "beatloop_%1_enabled", size));
    m_pEnabled->setReadOnly();
}

BeatLoopingControl::~BeatLoopingControl() {
    delete m_pActivate;
    delete m_pToggle;
    delete m_pEnabled;
    delete m_pLegacy;
    delete m_pActivateRoll;
}

void BeatLoopingControl::deactivate() {
    if (m_bActive) {
        m_bActive = false;
        m_pEnabled->forceSet(0);
        m_pLegacy->set(0);
    }
}

void BeatLoopingControl::activate() {
    if (!m_bActive) {
        m_bActive = true;
        m_pEnabled->forceSet(1);
        m_pLegacy->set(1);
    }
}

void BeatLoopingControl::slotLegacy(double v) {
    //qDebug() << "slotLegacy" << m_dBeatLoopSize << "v" << v;
    if (v > 0) {
        emit(activateBeatLoop(this));
    } else {
        emit(deactivateBeatLoop(this));
    }
}

void BeatLoopingControl::slotActivate(double v) {
    //qDebug() << "slotActivate" << m_dBeatLoopSize << "v" << v;
    if (!v) {
        return;
    }
    emit(activateBeatLoop(this));
}

void BeatLoopingControl::slotActivateRoll(double v) {
    //qDebug() << "slotActivateRoll" << m_dBeatLoopSize << "v" << v;
    if (v > 0) {
        emit(activateBeatLoopRoll(this));
    } else {
        emit(deactivateBeatLoopRoll(this));
    }
}

void BeatLoopingControl::slotToggle(double v) {
    //qDebug() << "slotToggle" << m_dBeatLoopSize << "v" << v;
    if (!v) {
        return;
    }
    if (m_bActive) {
        emit(deactivateBeatLoop(this));
    } else {
        emit(activateBeatLoop(this));
    }
}
