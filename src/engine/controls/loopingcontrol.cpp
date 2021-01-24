#include "engine/controls/loopingcontrol.h"

#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/controls/bpmcontrol.h"
#include "engine/controls/enginecontrol.h"
#include "engine/enginebuffer.h"
#include "moc_loopingcontrol.cpp"
#include "preferences/usersettings.h"
#include "track/track.h"
#include "util/compatibility.h"
#include "util/math.h"
#include "util/sample.h"

double LoopingControl::s_dBeatSizes[] = { 0.03125, 0.0625, 0.125, 0.25, 0.5,
                                          1, 2, 4, 8, 16, 32, 64, 128, 256, 512 };

// Used to generate the beatloop_%SIZE, beatjump_%SIZE, and loop_move_%SIZE CO
// ConfigKeys.
ConfigKey keyForControl(const QString& group, const QString& ctrlName, double num) {
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

LoopingControl::LoopingControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_bLoopingEnabled(false),
          m_bLoopRollActive(false),
          m_bAdjustingLoopIn(false),
          m_bAdjustingLoopOut(false),
          m_bAdjustingLoopInOld(false),
          m_bAdjustingLoopOutOld(false),
          m_bLoopOutPressedWhileLoopDisabled(false) {
    m_oldLoopSamples = {kNoTrigger, kNoTrigger, LoopSeekMode::MovedOut};
    m_loopSamples.setValue(m_oldLoopSamples);
    m_currentSample.setValue(0.0);
    m_pActiveBeatLoop = nullptr;
    m_pRateControl = nullptr;
    //Create loop-in, loop-out, loop-exit, and reloop/exit ControlObjects
    m_pLoopInButton = new ControlPushButton(ConfigKey(group, "loop_in"));
    connect(m_pLoopInButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopIn,
            Qt::DirectConnection);
    m_pLoopInButton->set(0);

    m_pLoopInGotoButton = new ControlPushButton(ConfigKey(group, "loop_in_goto"));
    connect(m_pLoopInGotoButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopInGoto);

    m_pLoopOutButton = new ControlPushButton(ConfigKey(group, "loop_out"));
    connect(m_pLoopOutButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopOut,
            Qt::DirectConnection);
    m_pLoopOutButton->set(0);

    m_pLoopOutGotoButton = new ControlPushButton(ConfigKey(group, "loop_out_goto"));
    connect(m_pLoopOutGotoButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopOutGoto);


    m_pLoopExitButton = new ControlPushButton(ConfigKey(group, "loop_exit"));
    connect(m_pLoopExitButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopExit,
            Qt::DirectConnection);
    m_pLoopExitButton->set(0);

    m_pReloopToggleButton = new ControlPushButton(ConfigKey(group, "reloop_toggle"));
    connect(m_pReloopToggleButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotReloopToggle,
            Qt::DirectConnection);
    m_pReloopToggleButton->set(0);
    // The old reloop_exit name was confusing. This CO does both entering and exiting.
    ControlDoublePrivate::insertAlias(ConfigKey(group, "reloop_exit"),
                                      ConfigKey(group, "reloop_toggle"));

    m_pReloopAndStopButton = new ControlPushButton(ConfigKey(group, "reloop_andstop"));
    connect(m_pReloopAndStopButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotReloopAndStop,
            Qt::DirectConnection);

    m_pCOLoopEnabled = new ControlObject(ConfigKey(group, "loop_enabled"));
    m_pCOLoopEnabled->set(0.0);
    m_pCOLoopEnabled->connectValueChangeRequest(this,
            &LoopingControl::slotLoopEnabledValueChangeRequest,
            Qt::DirectConnection);

    m_pCOLoopStartPosition =
            new ControlObject(ConfigKey(group, "loop_start_position"));
    m_pCOLoopStartPosition->set(kNoTrigger);
    connect(m_pCOLoopStartPosition, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopStartPos,
            Qt::DirectConnection);

    m_pCOLoopEndPosition =
            new ControlObject(ConfigKey(group, "loop_end_position"));
    m_pCOLoopEndPosition->set(kNoTrigger);
    connect(m_pCOLoopEndPosition, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopEndPos,
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
    connect(m_pCOBeatLoop, &ControlObject::valueChanged, this,
            [=](double value){slotBeatLoop(value);}, Qt::DirectConnection);

    m_pCOBeatLoopSize = new ControlObject(ConfigKey(group, "beatloop_size"),
                                          true, false, false, 4.0);
    m_pCOBeatLoopSize->connectValueChangeRequest(this,
            &LoopingControl::slotBeatLoopSizeChangeRequest, Qt::DirectConnection);
    m_pCOBeatLoopActivate = new ControlPushButton(ConfigKey(group, "beatloop_activate"));
    connect(m_pCOBeatLoopActivate, &ControlObject::valueChanged,
            this, &LoopingControl::slotBeatLoopToggle);
    m_pCOBeatLoopRollActivate = new ControlPushButton(ConfigKey(group, "beatlooproll_activate"));
    connect(m_pCOBeatLoopRollActivate, &ControlObject::valueChanged,
            this, &LoopingControl::slotBeatLoopRollActivate);

    // Here we create corresponding beatloop_(SIZE) CO's which all call the same
    // BeatControl, but with a set value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        BeatLoopingControl* pBeatLoop = new BeatLoopingControl(group, s_dBeatSizes[i]);
        connect(pBeatLoop, &BeatLoopingControl::activateBeatLoop,
                this, &LoopingControl::slotBeatLoopActivate,
                Qt::DirectConnection);
        connect(pBeatLoop,  &BeatLoopingControl::activateBeatLoopRoll,
                this, &LoopingControl::slotBeatLoopActivateRoll,
                Qt::DirectConnection);
        connect(pBeatLoop,  &BeatLoopingControl::deactivateBeatLoop,
                this, &LoopingControl::slotBeatLoopDeactivate,
                Qt::DirectConnection);
        connect(pBeatLoop,  &BeatLoopingControl::deactivateBeatLoopRoll,
                this, &LoopingControl::slotBeatLoopDeactivateRoll,
                Qt::DirectConnection);
        m_beatLoops.append(pBeatLoop);
    }

    m_pCOBeatJump = new ControlObject(ConfigKey(group, "beatjump"), false);
    connect(m_pCOBeatJump, &ControlObject::valueChanged,
            this, &LoopingControl::slotBeatJump, Qt::DirectConnection);
    m_pCOBeatJumpSize = new ControlObject(ConfigKey(group, "beatjump_size"),
                                          true, false, false, 4.0);
    m_pCOBeatJumpForward = new ControlPushButton(ConfigKey(group, "beatjump_forward"));
    connect(m_pCOBeatJumpForward, &ControlObject::valueChanged,
            this, &LoopingControl::slotBeatJumpForward);
    m_pCOBeatJumpBackward = new ControlPushButton(ConfigKey(group, "beatjump_backward"));
    connect(m_pCOBeatJumpBackward, &ControlObject::valueChanged,
            this, &LoopingControl::slotBeatJumpBackward);

    // Create beatjump_(SIZE) CO's which all call beatjump, but with a set
    // value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        BeatJumpControl* pBeatJump = new BeatJumpControl(group, s_dBeatSizes[i]);
        connect(pBeatJump, &BeatJumpControl::beatJump,
                this, &LoopingControl::slotBeatJump,
                Qt::DirectConnection);
        m_beatJumps.append(pBeatJump);
    }

    m_pCOLoopMove = new ControlObject(ConfigKey(group, "loop_move"), false);
    connect(m_pCOLoopMove, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopMove, Qt::DirectConnection);

    // Create loop_move_(SIZE) CO's which all call loop_move, but with a set
    // value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        LoopMoveControl* pLoopMove = new LoopMoveControl(group, s_dBeatSizes[i]);
        connect(pLoopMove, &LoopMoveControl::loopMove,
                this, &LoopingControl::slotLoopMove,
                Qt::DirectConnection);
        m_loopMoves.append(pLoopMove);
    }

    m_pCOLoopScale = new ControlObject(ConfigKey(group, "loop_scale"), false);
    connect(m_pCOLoopScale, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopScale);
    m_pLoopHalveButton = new ControlPushButton(ConfigKey(group, "loop_halve"));
    connect(m_pLoopHalveButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopHalve);
    m_pLoopDoubleButton = new ControlPushButton(ConfigKey(group, "loop_double"));
    connect(m_pLoopDoubleButton, &ControlObject::valueChanged,
            this, &LoopingControl::slotLoopDouble);

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
    const double loopLength = (loopSamples.end - loopSamples.start) * scaleFactor;
    const int trackSamples = static_cast<int>(m_pTrackSamples->get());

    // Abandon loops that are too short of extend beyond the end of the file.
    if (loopLength < MINIMUM_AUDIBLE_LOOP_SIZE ||
            loopSamples.start + loopLength > trackSamples) {
        return;
    }

    loopSamples.end = loopSamples.start + loopLength;

    // TODO(XXX) we could be smarter about taking the active beatloop, scaling
    // it by the desired amount and trying to find another beatloop that matches
    // it, but for now we just clear the active beat loop if somebody scales.
    clearActiveBeatLoop();

    // Don't allow 0 samples loop, so one can still manipulate it
    if (loopSamples.end == loopSamples.start) {
        if ((loopSamples.end + 2) >= trackSamples) {
            loopSamples.start -= 2;
        } else {
            loopSamples.end += 2;
        }
    }
    // Do not allow loops to go past the end of the song
    else if (loopSamples.end > trackSamples) {
        loopSamples.end = trackSamples;
    }

    // Reseek if the loop shrank out from under the playposition.
    loopSamples.seekMode = (m_bLoopingEnabled && scaleFactor < 1.0)
            ? LoopSeekMode::Changed
            : LoopSeekMode::MovedOut;

    m_loopSamples.setValue(loopSamples);
    emit loopUpdated(loopSamples.start, loopSamples.end);

    // Update CO for loop end marker
    m_pCOLoopEndPosition->set(loopSamples.end);
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

void LoopingControl::process(const double dRate,
                             const double currentSample,
                             const int iBufferSize) {
    Q_UNUSED(iBufferSize);
    Q_UNUSED(dRate);

    double oldCurrentSample = m_currentSample.getValue();

    if (oldCurrentSample != currentSample) {
        m_currentSample.setValue(currentSample);
    } else {
        // no transport, so we have to do scheduled seeks here
        LoopSamples loopSamples = m_loopSamples.getValue();
        if (m_bLoopingEnabled &&
            !m_bAdjustingLoopIn && !m_bAdjustingLoopOut &&
            loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {

            if (loopSamples.start != m_oldLoopSamples.start ||
                    loopSamples.end != m_oldLoopSamples.end) {
                // bool seek is only valid after the loop has changed
                if (loopSamples.seekMode == LoopSeekMode::Changed) {
                    // here the loop has changed and the play position
                    // should be moved with it
                    double target = seekInsideAdjustedLoop(currentSample,
                            m_oldLoopSamples.start, loopSamples.start, loopSamples.end);
                    if (target != kNoTrigger) {
                        // jump immediately
                        seekAbs(target);
                    }
                }
                m_oldLoopSamples = loopSamples;
            }
        }
    }

    if (m_bAdjustingLoopIn) {
        setLoopInToCurrentPosition();
    } else if (m_bAdjustingLoopOut) {
        setLoopOutToCurrentPosition();
    }
}

double LoopingControl::nextTrigger(bool reverse,
        const double currentSample,
        double *pTarget) {
    *pTarget = kNoTrigger;

    LoopSamples loopSamples = m_loopSamples.getValue();

    // m_bAdjustingLoopIn is true while the LoopIn button is pressed while a loop is active (slotLoopIn)
    if (m_bAdjustingLoopInOld != m_bAdjustingLoopIn) {
        m_bAdjustingLoopInOld = m_bAdjustingLoopIn;

        // When the LoopIn button is released in reverse mode we jump to the end of the loop to not fall out and disable the active loop
        // This must not happen in quantized mode. The newly set start is always ahead (in time, but behind spacially) of the current position so we don't jump.
        // Jumping to the end is then handled when the loop's start is reached later in this function.
        if (reverse && !m_bAdjustingLoopIn && !m_pQuantizeEnabled->toBool()) {
            m_oldLoopSamples = loopSamples;
            *pTarget = loopSamples.end;
            return currentSample;
        }
    }

    // m_bAdjustingLoopOut is true while the LoopOut button is pressed while a loop is active (slotLoopOut)
    if (m_bAdjustingLoopOutOld != m_bAdjustingLoopOut) {
        m_bAdjustingLoopOutOld = m_bAdjustingLoopOut;

        // When the LoopOut button is released in forward mode we jump to the start of the loop to not fall out and disable the active loop
        // This must not happen in quantized mode. The newly set end is always ahead of the current position so we don't jump.
        // Jumping to the start is then handled when the loop's end is reached later in this function.
        if (!reverse && !m_bAdjustingLoopOut && !m_pQuantizeEnabled->toBool()) {
            m_oldLoopSamples = loopSamples;
            *pTarget = loopSamples.start;
            return currentSample;
        }
    }

    if (m_bLoopingEnabled &&
            !m_bAdjustingLoopIn && !m_bAdjustingLoopOut &&
            loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {

        if (loopSamples.start != m_oldLoopSamples.start ||
                loopSamples.end != m_oldLoopSamples.end) {
            // bool seek is only valid after the loop has changed
            switch (loopSamples.seekMode) {
            case LoopSeekMode::Changed:
                // here the loop has changed and the play position
                // should be moved with it
                *pTarget = seekInsideAdjustedLoop(currentSample,
                        m_oldLoopSamples.start,
                        loopSamples.start,
                        loopSamples.end);
                break;
            case LoopSeekMode::MovedOut: {
                bool movedOut = false;
                // Check if we have moved out of the loop, before we could enable it
                if (reverse) {
                    if (loopSamples.start > currentSample) {
                        movedOut = true;
                    }
                } else {
                    if (loopSamples.end < currentSample) {
                        movedOut = true;
                    }
                }
                if (movedOut) {
                    *pTarget = seekInsideAdjustedLoop(currentSample,
                            loopSamples.start,
                            loopSamples.start,
                            loopSamples.end);
                }
                break;
            }
            case LoopSeekMode::None:
                // Nothing to do here. This is used for enabling saved loops
                // which we want to do without jumping to the loop start
                // position.
                break;
            }
            m_oldLoopSamples = loopSamples;
            if (*pTarget != kNoTrigger) {
                // jump immediately
                return currentSample;
            }
        }

        if (reverse) {
            *pTarget = loopSamples.end;
            return loopSamples.start;
        } else {
            *pTarget = loopSamples.start;
            return loopSamples.end;
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

double LoopingControl::getSyncPositionInsideLoop(double dRequestedPlaypos, double dSyncedPlayPos) {
    // no loop, no adjustment
    if (!m_bLoopingEnabled) {
        return dSyncedPlayPos;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();

    // if the request itself is outside loop do nothing
    // loop will be disabled later by notifySeek(...) as is was explicitly requested by the user
    // if the requested position is the exact end of a loop it should also be disabled later by notifySeek(...)
    if (dRequestedPlaypos < loopSamples.start || dRequestedPlaypos >= loopSamples.end) {
        return dSyncedPlayPos;
    }

    // the requested position is inside the loop (e.g hotcue at start)
    double loopSize = loopSamples.end - loopSamples.start;

    // the synced position is in front of the loop
    // adjust the synced position to same amount in front of the loop end
    if (dSyncedPlayPos < loopSamples.start) {
        double adjustment = loopSamples.start - dSyncedPlayPos;

        // prevents jumping in front of the loop if loop is smaller than adjustment
        adjustment = fmod(adjustment, loopSize);

        // if the synced position is exactly the start of the loop we would end up at the exact end
        // as this would disable the loop in notifySeek() replace it with the start of the loop
        if (adjustment == 0) {
            return loopSamples.start;
        }
        return loopSamples.end - adjustment;
    }

    // the synced position is behind the loop
    // adjust the synced position to same amount behind the loop start
    if (dSyncedPlayPos >= loopSamples.end) {
        double adjustment = dSyncedPlayPos - loopSamples.end;

        // prevents jumping behind the loop if loop is smaller than adjustment
        adjustment = fmod(adjustment, loopSize);

        return loopSamples.start + adjustment;
    }

    // both, requested and synced position are inside the loop -> do nothing
    return dSyncedPlayPos;
}

void LoopingControl::setBeatLoop(double startPosition, bool enabled) {
    VERIFY_OR_DEBUG_ASSERT(startPosition != Cue::kNoPosition) {
        return;
    }

    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return;
    }

    double beatloopSize = m_pCOBeatLoopSize->get();

    // TODO(XXX): This is not realtime safe. See this Zulip discussion for details:
    // https://mixxx.zulipchat.com/#narrow/stream/109171-development/topic/getting.20locks.20out.20of.20Beats
    double endPosition = pBeats->findNBeatsFromSample(startPosition, beatloopSize);

    setLoop(startPosition, endPosition, enabled);
}

void LoopingControl::setLoop(double startPosition, double endPosition, bool enabled) {
    VERIFY_OR_DEBUG_ASSERT(startPosition != Cue::kNoPosition &&
            endPosition != Cue::kNoPosition && startPosition < endPosition) {
        return;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start != startPosition || loopSamples.end != endPosition) {
        // Copy saved loop parameters to active loop
        loopSamples.start = startPosition;
        loopSamples.end = endPosition;
        loopSamples.seekMode = LoopSeekMode::None;
        clearActiveBeatLoop();
        m_loopSamples.setValue(loopSamples);
        m_pCOLoopStartPosition->set(loopSamples.start);
        m_pCOLoopEndPosition->set(loopSamples.end);
    }
    setLoopingEnabled(enabled);

    // Seek back to loop in position if we're already behind the loop end.
    //
    // TODO(Holzhaus): This needs to be reverted as soon as GUI controls for
    // controlling saved loop behaviour are in place, because this change makes
    // saved loops very risky to use and might potentially mess up your mix.
    // See https://github.com/mixxxdj/mixxx/pull/2194#issuecomment-721847833
    // for details.
    if (enabled && m_currentSample.getValue() > loopSamples.end) {
        slotLoopInGoto(1);
    }

    m_pCOBeatLoopSize->setAndConfirm(findBeatloopSizeForLoop(startPosition, endPosition));
}

void LoopingControl::setLoopInToCurrentPosition() {
    // set loop-in position
    mixxx::BeatsPointer pBeats = m_pBeats;
    LoopSamples loopSamples = m_loopSamples.getValue();
    double quantizedBeat = -1;
    double pos = m_currentSample.getValue();
    if (m_pQuantizeEnabled->toBool() && pBeats) {
        if (m_bAdjustingLoopIn) {
            double closestBeat = m_pClosestBeat->get();
            if (closestBeat == m_currentSample.getValue()) {
                quantizedBeat = closestBeat;
            } else {
                quantizedBeat = m_pPreviousBeat->get();
            }
        } else {
            quantizedBeat = m_pClosestBeat->get();
        }
        if (quantizedBeat != -1) {
            pos = quantizedBeat;
        }
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
        if (quantizedBeat != -1 && pBeats) {
            pos = pBeats->findNthBeat(quantizedBeat, -2);
            if (pos == -1 || (loopSamples.end - pos) < MINIMUM_AUDIBLE_LOOP_SIZE) {
                pos = loopSamples.end - MINIMUM_AUDIBLE_LOOP_SIZE;
            }
        } else {
            pos = loopSamples.end - MINIMUM_AUDIBLE_LOOP_SIZE;
        }
    }

    loopSamples.start = pos;

    m_pCOLoopStartPosition->set(loopSamples.start);

    // start looping
    if (loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {
        setLoopingEnabled(true);
        loopSamples.seekMode = LoopSeekMode::Changed;
    } else {
        loopSamples.seekMode = LoopSeekMode::MovedOut;
    }

    if (m_pQuantizeEnabled->toBool()
            && loopSamples.start < loopSamples.end
            && pBeats) {
        m_pCOBeatLoopSize->setAndConfirm(
                pBeats->numBeatsInRange(loopSamples.start, loopSamples.end));
        updateBeatLoopingControls();
    } else {
        clearActiveBeatLoop();
    }

    m_loopSamples.setValue(loopSamples);
    //qDebug() << "set loop_in to " << loopSamples.start;
}

void LoopingControl::slotLoopIn(double pressed) {
    if (!m_pTrack) {
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
            LoopSamples loopSamples = m_loopSamples.getValue();
            if (loopSamples.start < loopSamples.end) {
                emit loopUpdated(loopSamples.start, loopSamples.end);
            } else {
                emit loopReset();
            }
        }
    } else {
        emit loopReset();
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
    mixxx::BeatsPointer pBeats = m_pBeats;
    LoopSamples loopSamples = m_loopSamples.getValue();
    double quantizedBeat = -1;
    double pos = m_currentSample.getValue();
    if (m_pQuantizeEnabled->toBool() && pBeats) {
        if (m_bAdjustingLoopOut) {
            double closestBeat = m_pClosestBeat->get();
            if (closestBeat == m_currentSample.getValue()) {
                quantizedBeat = closestBeat;
            } else {
                quantizedBeat = m_pNextBeat->get();
            }
        } else {
            quantizedBeat = m_pClosestBeat->get();
        }
        if (quantizedBeat != -1) {
            pos = quantizedBeat;
        }
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
        if (quantizedBeat != -1 && pBeats) {
            pos = static_cast<int>(floor(pBeats->findNthBeat(quantizedBeat, 2)));
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

    // start looping
    if (loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {
        setLoopingEnabled(true);
        loopSamples.seekMode = LoopSeekMode::Changed;
    } else {
        loopSamples.seekMode = LoopSeekMode::MovedOut;
    }

    if (m_pQuantizeEnabled->toBool() && pBeats) {
        m_pCOBeatLoopSize->setAndConfirm(
            pBeats->numBeatsInRange(loopSamples.start, loopSamples.end));
        updateBeatLoopingControls();
    } else {
        clearActiveBeatLoop();
    }
    //qDebug() << "set loop_out to " << loopSamples.end;

    m_loopSamples.setValue(loopSamples);
}

void LoopingControl::setRateControl(RateControl* rateControl) {
    m_pRateControl = rateControl;
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
                LoopSamples loopSamples = m_loopSamples.getValue();
                if (loopSamples.start < loopSamples.end) {
                    emit loopUpdated(loopSamples.start, loopSamples.end);
                } else {
                    emit loopReset();
                }
                m_bAdjustingLoopOut = false;
            } else {
                m_bLoopOutPressedWhileLoopDisabled = false;
            }
        }
    } else {
        emit loopReset();
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

void LoopingControl::slotLoopEnabledValueChangeRequest(double value) {
    if (!m_pTrack) {
        return;
    }

    if (value > 0.0) {
        // Requested to set loop_enabled to 1
        if (m_bLoopingEnabled) {
            VERIFY_OR_DEBUG_ASSERT(m_pCOLoopEnabled->toBool()) {
                m_pCOLoopEnabled->setAndConfirm(1.0);
            }
        } else {
            // Looping is currently disabled, try to enable the loop. In
            // contrast to the reloop_toggle CO, we jump in no case.
            LoopSamples loopSamples = m_loopSamples.getValue();
            if (loopSamples.start != kNoTrigger && loopSamples.end != kNoTrigger &&
                    loopSamples.start <= loopSamples.end) {
                // setAndConfirm is called by setLoopingEnabled
                setLoopingEnabled(true);
            }
        }
    } else {
        // Requested to set loop_enabled to 0
        if (m_bLoopingEnabled) {
            // Looping is currently enabled, disable the loop. If loop roll
            // was active, also disable slip.
            if (m_bLoopRollActive) {
                m_pSlipEnabled->set(0);
                m_bLoopRollActive = false;
                m_activeLoopRolls.clear();
            }
            // setAndConfirm is called by setLoopingEnabled
            setLoopingEnabled(false);
        } else {
            VERIFY_OR_DEBUG_ASSERT(!m_pCOLoopEnabled->toBool()) {
                m_pCOLoopEnabled->setAndConfirm(0.0);
            }
        }
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
            m_activeLoopRolls.clear();
        }
        setLoopingEnabled(false);
        //qDebug() << "reloop_toggle looping off";
    } else {
        // If we're not looping, enable the loop. If the loop is ahead of the
        // current play position, do not jump to it.
        LoopSamples loopSamples = m_loopSamples.getValue();
        if (loopSamples.start != kNoTrigger && loopSamples.end != kNoTrigger &&
                loopSamples.start <= loopSamples.end) {
            setLoopingEnabled(true);
            if (m_currentSample.getValue() > loopSamples.end) {
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
    // This slot is called before trackLoaded() for a new Track
    LoopSamples loopSamples = m_loopSamples.getValue();

    if (loopSamples.start == pos) {
        //nothing to do
        return;
    }

    clearActiveBeatLoop();

    if (pos == kNoTrigger) {
        emit loopReset();
        setLoopingEnabled(false);
    }

    loopSamples.seekMode = LoopSeekMode::MovedOut;
    loopSamples.start = pos;
    m_pCOLoopStartPosition->set(pos);

    if (loopSamples.end != kNoTrigger &&
            loopSamples.end <= loopSamples.start) {
        emit loopReset();
        loopSamples.end = kNoTrigger;
        m_pCOLoopEndPosition->set(kNoTrigger);
        setLoopingEnabled(false);
    }
    m_loopSamples.setValue(loopSamples);
}

void LoopingControl::slotLoopEndPos(double pos) {
    // This slot is called before trackLoaded() for a new Track

    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.end == pos) {
        //nothing to do
        return;
    }

    // Reject if the loop-in is not set, or if the new position is before the
    // start point (but not -1).
    if (loopSamples.start == kNoTrigger ||
            (pos != kNoTrigger && pos <= loopSamples.start)) {
        m_pCOLoopEndPosition->set(loopSamples.end);
        return;
    }

    clearActiveBeatLoop();

    if (pos == kNoTrigger) {
        emit loopReset();
        setLoopingEnabled(false);
    }
    loopSamples.end = pos;
    loopSamples.seekMode = LoopSeekMode::MovedOut;
    m_pCOLoopEndPosition->set(pos);
    m_loopSamples.setValue(loopSamples);
}

// This is called from the engine thread
void LoopingControl::notifySeek(double dNewPlaypos) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    double currentSample = m_currentSample.getValue();
    if (m_bLoopingEnabled) {
        // Disable loop when we jumping out, or over a catching loop,
        // using hot cues or waveform overview.
        // Jumping to the exact end of a loop is considered jumping out.
        if (currentSample >= loopSamples.start &&
                currentSample <= loopSamples.end &&
                dNewPlaypos < loopSamples.start) {
            // jumping out of loop in backwards
            setLoopingEnabled(false);
        }
        if (currentSample <= loopSamples.end &&
                dNewPlaypos >= loopSamples.end) {
            // jumping out or to the exact end of a loop or over a catching loop forward
            setLoopingEnabled(false);
        }
    }
    EngineControl::notifySeek(dNewPlaypos);
}

void LoopingControl::setLoopingEnabled(bool enabled) {
    if (m_bLoopingEnabled == enabled) {
        return;
    }

    m_bLoopingEnabled = enabled;
    m_pCOLoopEnabled->setAndConfirm(enabled ? 1.0 : 0.0);
    BeatLoopingControl* pActiveBeatLoop = atomicLoadRelaxed(m_pActiveBeatLoop);
    if (pActiveBeatLoop != nullptr) {
        if (enabled) {
            pActiveBeatLoop->activate();
        } else {
            pActiveBeatLoop->deactivate();
        }
    }

    emit loopEnabledChanged(enabled);
}

bool LoopingControl::isLoopingEnabled() {
    return m_bLoopingEnabled;
}

void LoopingControl::trackLoaded(TrackPointer pNewTrack) {
    m_pTrack = pNewTrack;
    mixxx::BeatsPointer pBeats;
    if (pNewTrack) {
        pBeats = pNewTrack->getBeats();
    }
    trackBeatsUpdated(pBeats);
}

void LoopingControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    clearActiveBeatLoop();
    m_pBeats = pBeats;
    if (m_pBeats) {
        LoopSamples loopSamples = m_loopSamples.getValue();
        if (loopSamples.start != kNoTrigger && loopSamples.end != kNoTrigger) {
            double loaded_loop_size = findBeatloopSizeForLoop(
                loopSamples.start, loopSamples.end);
            if (loaded_loop_size != -1) {
                m_pCOBeatLoopSize->setAndConfirm(loaded_loop_size);
            }
        }
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

    // Disregard existing loops (except beatlooprolls).
    m_pSlipEnabled->set(1);
    slotBeatLoop(pBeatLoopControl->getSize(), m_bLoopRollActive, true);
    m_bLoopRollActive = true;
    m_activeLoopRolls.push(pBeatLoopControl->getSize());
}

void LoopingControl::slotBeatLoopDeactivate(BeatLoopingControl* pBeatLoopControl) {
    Q_UNUSED(pBeatLoopControl);
    setLoopingEnabled(false);
}

void LoopingControl::slotBeatLoopDeactivateRoll(BeatLoopingControl* pBeatLoopControl) {
    pBeatLoopControl->deactivate();
    const double size = pBeatLoopControl->getSize();
    auto* i = m_activeLoopRolls.begin();
    while (i != m_activeLoopRolls.end()) {
        if (size == *i) {
            i = m_activeLoopRolls.erase(i);
        } else {
            ++i;
        }
    }

    // Make sure slip mode is not turned off if it was turned on
    // by something that was not a rolling beatloop.
    if (m_bLoopRollActive && m_activeLoopRolls.empty()) {
        setLoopingEnabled(false);
        m_pSlipEnabled->set(0);
        m_bLoopRollActive = false;
    }

    // Return to the previous beatlooproll if necessary.
    if (!m_activeLoopRolls.empty()) {
        slotBeatLoop(m_activeLoopRolls.top(), m_bLoopRollActive, true);
    }
}

void LoopingControl::clearActiveBeatLoop() {
    BeatLoopingControl* pOldBeatLoop = m_pActiveBeatLoop.fetchAndStoreAcquire(nullptr);
    if (pOldBeatLoop != nullptr) {
        pOldBeatLoop->deactivate();
    }
}

bool LoopingControl::currentLoopMatchesBeatloopSize() {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return false;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();

    // Calculate where the loop out point would be if it is a beatloop
    double beatLoopOutPoint =
        pBeats->findNBeatsFromSample(loopSamples.start, m_pCOBeatLoopSize->get());

    return loopSamples.end > beatLoopOutPoint - 2 &&
            loopSamples.end < beatLoopOutPoint + 2;
}

double LoopingControl::findBeatloopSizeForLoop(double start, double end) const {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return -1;
    }

    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        double beatLoopOutPoint =
            pBeats->findNBeatsFromSample(start, s_dBeatSizes[i]);
        if (end > beatLoopOutPoint - 2 && end < beatLoopOutPoint + 2) {
            return s_dBeatSizes[i];
        }
    }
    return -1;
}

void LoopingControl::updateBeatLoopingControls() {
    // O(n) search, but there are only ~10-ish beatloop controls so this is
    // fine.
    double dBeatloopSize = m_pCOBeatLoopSize->get();
    for (BeatLoopingControl* pBeatLoopControl: qAsConst(m_beatLoops)) {
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
    // If this is a "new" loop, stop tracking saved loop changes
    if (!keepStartPoint) {
        emit loopReset();
    }

    // if a seek was queued in the engine buffer move the current sample to its position
    double p_seekPosition = 0;
    if (getEngineBuffer()->getQueuedSeekPosition(&p_seekPosition)) {
        // seek position is already quantized if quantization is enabled
        m_currentSample.setValue(p_seekPosition);
    }

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

    int samples = static_cast<int>(m_pTrackSamples->get());
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (samples == 0 || !pBeats) {
        clearActiveBeatLoop();
        m_pCOBeatLoopSize->setAndConfirm(beats);
        return;
    }

    // Calculate the new loop start and end samples
    // give start and end defaults so we can detect problems
    LoopSamples newloopSamples = {kNoTrigger, kNoTrigger, LoopSeekMode::MovedOut};
    LoopSamples loopSamples = m_loopSamples.getValue();
    double currentSample = m_currentSample.getValue();

    // Start from the current position/closest beat and
    // create the loop around X beats from there.
    if (keepStartPoint) {
        if (loopSamples.start != kNoTrigger) {
            newloopSamples.start = loopSamples.start;
        } else {
            newloopSamples.start = currentSample;
        }
    } else {
        // loop_in is set to the closest beat if quantize is on and the loop size is >= 1 beat.
        // The closest beat might be ahead of play position and will cause a catching loop.
        double prevBeat;
        double nextBeat;
        pBeats->findPrevNextBeats(currentSample, &prevBeat, &nextBeat);

        if (m_pQuantizeEnabled->toBool() && prevBeat != -1) {
            double beatLength = nextBeat - prevBeat;
            double loopLength = beatLength * beats;

            double closestBeat = pBeats->findClosestBeat(currentSample);
            if (beats >= 1.0) {
                newloopSamples.start = closestBeat;
            } else {
                // In case of beat length less then 1 beat:
                // (| - beats, ^ - current track's position):
                //
                // ...|...................^........|...
                //
                // If we press 1/2 beatloop we want loop from 50% to 100%,
                // If I press 1/4 beatloop, we want loop from 50% to 75% etc
                double samplesSinceLastBeat = currentSample - prevBeat;

                // find the previous beat fraction and check if the current position is closer to this or the next one
                // place the new loop start to the closer one
                double previousFractionBeat = prevBeat + floor(samplesSinceLastBeat / loopLength) * loopLength;
                double samplesSinceLastFractionBeat = currentSample - previousFractionBeat;

                if (samplesSinceLastFractionBeat <= (loopLength / 2.0)) {
                    newloopSamples.start = previousFractionBeat;
                } else {
                    newloopSamples.start = previousFractionBeat + loopLength;
                }
            }

            // If running reverse, move the loop one loop size to the left.
            // Thus, the loops end will be closest to the current position
            bool reverse = false;
            if (m_pRateControl != nullptr) {
                reverse = m_pRateControl->isReverseButtonPressed();
            }
            if (reverse) {
                newloopSamples.start -= loopLength;
            }
        } else {
            newloopSamples.start = currentSample;
        }
    }

    newloopSamples.end = pBeats->findNBeatsFromSample(newloopSamples.start, beats);
    if (newloopSamples.start >= newloopSamples.end // happens when the call above fails
            || newloopSamples.end > samples) { // Do not allow beat loops to go beyond the end of the track
        // If a track is loaded with beatloop_size larger than
        // the distance between the loop in point and
        // the end of the track, let beatloop_size be set to
        // a smaller size, but not get larger.
        double previousBeatloopSize = m_pCOBeatLoopSize->get();
        double previousBeatloopOutPoint = pBeats->findNBeatsFromSample(
                newloopSamples.start, previousBeatloopSize);
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
    bool omitResize = false;
    if (!currentLoopMatchesBeatloopSize() && !enable) {
        omitResize = true;
    }

    if (m_pCOBeatLoopSize->get() != beats) {
        m_pCOBeatLoopSize->setAndConfirm(beats);
    }

    // This check happens after setting m_pCOBeatLoopSize so
    // beatloop_size can be prepared without having a track loaded.
    if ((newloopSamples.start == kNoTrigger) || (newloopSamples.end == kNoTrigger)) {
        return;
    }

    if (omitResize) {
        return;
    }

    // If resizing an inactive loop by changing beatloop_size,
    // do not seek to the adjusted loop.
    newloopSamples.seekMode = (keepStartPoint && (enable || m_bLoopingEnabled))
            ? LoopSeekMode::Changed
            : LoopSeekMode::MovedOut;

    m_loopSamples.setValue(newloopSamples);
    emit loopUpdated(newloopSamples.start, newloopSamples.end);
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
                m_activeLoopRolls.clear();
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
            m_activeLoopRolls.clear();
        }
    }
}

void LoopingControl::slotBeatJump(double beats) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats) {
        return;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();
    double currentSample = m_currentSample.getValue();

    if (m_bLoopingEnabled && !m_bAdjustingLoopIn && !m_bAdjustingLoopOut &&
            loopSamples.start <= currentSample &&
            loopSamples.end >= currentSample) {
        // If inside an active loop, move loop
        slotLoopMove(beats);
    } else {
        // seekExact bypasses Quantize, because a beat jump is implicit quantized
        seekExact(pBeats->findNBeatsFromSample(currentSample, beats));
    }
}

void LoopingControl::slotBeatJumpForward(double pressed) {
    if (pressed != 0) {
        slotBeatJump(m_pCOBeatJumpSize->get());
    }
}

void LoopingControl::slotBeatJumpBackward(double pressed) {
    if (pressed != 0) {
        slotBeatJump(-1.0 * m_pCOBeatJumpSize->get());
    }
}

void LoopingControl::slotLoopMove(double beats) {
    mixxx::BeatsPointer pBeats = m_pBeats;
    if (!pBeats || beats == 0) {
        return;
    }
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || loopSamples.end == kNoTrigger) {
        return;
    }

    if (BpmControl::getBeatContext(pBeats, m_currentSample.getValue(),
                                   nullptr, nullptr, nullptr, nullptr)) {
        double new_loop_in = pBeats->findNBeatsFromSample(loopSamples.start, beats);
        double new_loop_out = currentLoopMatchesBeatloopSize() ?
                pBeats->findNBeatsFromSample(new_loop_in, m_pCOBeatLoopSize->get()) :
                pBeats->findNBeatsFromSample(loopSamples.end, beats);

        // The track would stop as soon as the playhead crosses track end,
        // so we don't allow moving a loop beyond end.
        // https://bugs.launchpad.net/mixxx/+bug/1799574
        if (new_loop_out > m_pTrackSamples->get()) {
            return;
        }
        // If we are looping make sure that the play head does not leave the
        // loop as a result of our adjustment.
        loopSamples.seekMode = m_bLoopingEnabled ? LoopSeekMode::Changed : LoopSeekMode::MovedOut;

        loopSamples.start = new_loop_in;
        loopSamples.end = new_loop_out;
        m_loopSamples.setValue(loopSamples);
        emit loopUpdated(loopSamples.start, loopSamples.end);
        m_pCOLoopStartPosition->set(new_loop_in);
        m_pCOLoopEndPosition->set(new_loop_out);
    }
}

// Must be called from the engine thread only
double LoopingControl::seekInsideAdjustedLoop(
        double currentSample, double old_loop_in,
        double new_loop_in, double new_loop_out) {
    if (currentSample >= new_loop_in && currentSample <= new_loop_out) {
        // playposition already is inside the loop
        return kNoTrigger;
    }
    if (currentSample < old_loop_in && currentSample <= new_loop_out) {
        // Playposition was before a catching loop and is still a catching loop
        // nothing to do
        return kNoTrigger;
    }

    double new_loop_size = new_loop_out - new_loop_in;
    DEBUG_ASSERT(new_loop_size > 0);
    double adjusted_position = currentSample;
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
        return adjusted_position;
    } else {
        return kNoTrigger;
    }
}

BeatJumpControl::BeatJumpControl(const QString& group, double size)
        : m_dBeatJumpSize(size) {
    m_pJumpForward = new ControlPushButton(
            keyForControl(group, "beatjump_%1_forward", size));
    connect(m_pJumpForward, &ControlObject::valueChanged,
            this, &BeatJumpControl::slotJumpForward,
            Qt::DirectConnection);
    m_pJumpBackward = new ControlPushButton(
            keyForControl(group, "beatjump_%1_backward", size));
    connect(m_pJumpBackward, &ControlObject::valueChanged,
            this, &BeatJumpControl::slotJumpBackward,
            Qt::DirectConnection);
}

BeatJumpControl::~BeatJumpControl() {
    delete m_pJumpForward;
    delete m_pJumpBackward;
}

void BeatJumpControl::slotJumpBackward(double pressed) {
    if (pressed > 0) {
        emit beatJump(-m_dBeatJumpSize);
    }
}

void BeatJumpControl::slotJumpForward(double pressed) {
    if (pressed > 0) {
        emit beatJump(m_dBeatJumpSize);
    }
}

LoopMoveControl::LoopMoveControl(const QString& group, double size)
        : m_dLoopMoveSize(size) {
    m_pMoveForward = new ControlPushButton(
            keyForControl(group, "loop_move_%1_forward", size));
    connect(m_pMoveForward, &ControlObject::valueChanged,
            this, &LoopMoveControl::slotMoveForward,
            Qt::DirectConnection);
    m_pMoveBackward = new ControlPushButton(
            keyForControl(group, "loop_move_%1_backward", size));
    connect(m_pMoveBackward, &ControlObject::valueChanged,
            this, &LoopMoveControl::slotMoveBackward,
            Qt::DirectConnection);
}

LoopMoveControl::~LoopMoveControl() {
    delete m_pMoveForward;
    delete m_pMoveBackward;
}

void LoopMoveControl::slotMoveBackward(double v) {
    if (v > 0) {
        emit loopMove(-m_dLoopMoveSize);
    }
}

void LoopMoveControl::slotMoveForward(double v) {
    if (v > 0) {
        emit loopMove(m_dLoopMoveSize);
    }
}

BeatLoopingControl::BeatLoopingControl(const QString& group, double size)
        : m_dBeatLoopSize(size),
          m_bActive(false) {
    // This is the original beatloop control which is now deprecated. Its value
    // is the state of the beatloop control (1 for enabled, 0 for disabled).
    m_pLegacy = new ControlPushButton(
            keyForControl(group, "beatloop_%1", size));
    m_pLegacy->setButtonMode(ControlPushButton::TOGGLE);
    connect(m_pLegacy, &ControlObject::valueChanged,
            this, &BeatLoopingControl::slotLegacy,
            Qt::DirectConnection);
    // A push-button which activates the beatloop.
    m_pActivate = new ControlPushButton(
            keyForControl(group, "beatloop_%1_activate", size));
    connect(m_pActivate, &ControlObject::valueChanged,
            this, &BeatLoopingControl::slotActivate,
            Qt::DirectConnection);
    // A push-button which toggles the beatloop as active or inactive.
    m_pToggle = new ControlPushButton(
            keyForControl(group, "beatloop_%1_toggle", size));
    connect(m_pToggle, &ControlObject::valueChanged,
            this, &BeatLoopingControl::slotToggle,
            Qt::DirectConnection);

    // A push-button which activates rolling beatloops
    m_pActivateRoll = new ControlPushButton(
            keyForControl(group, "beatlooproll_%1_activate", size));
    connect(m_pActivateRoll, &ControlObject::valueChanged,
            this, &BeatLoopingControl::slotActivateRoll,
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
        emit activateBeatLoop(this);
    } else {
        emit deactivateBeatLoop(this);
    }
}

void BeatLoopingControl::slotActivate(double value) {
    //qDebug() << "slotActivate" << m_dBeatLoopSize << "value" << value;
    if (value == 0) {
        return;
    }
    emit activateBeatLoop(this);
}

void BeatLoopingControl::slotActivateRoll(double v) {
    //qDebug() << "slotActivateRoll" << m_dBeatLoopSize << "v" << v;
    if (v > 0) {
        emit activateBeatLoopRoll(this);
    } else {
        emit deactivateBeatLoopRoll(this);
    }
}

void BeatLoopingControl::slotToggle(double value) {
    //qDebug() << "slotToggle" << m_dBeatLoopSize << "value" << value;
    if (value == 0) {
        return;
    }
    if (m_bActive) {
        emit deactivateBeatLoop(this);
    } else {
        emit activateBeatLoop(this);
    }
}
