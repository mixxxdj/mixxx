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

#include "track/track.h"
#include "track/beats.h"

double LoopingControl::s_dBeatSizes[] = { 0.03125, 0.0625, 0.125, 0.25, 0.5,
                                          1, 2, 4, 8, 16, 32, 64 };

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
    m_bLoopingEnabled = false;
    m_bLoopRollActive = false;
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

    m_pLoopOutButton = new ControlPushButton(ConfigKey(group, "loop_out"));
    connect(m_pLoopOutButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopOut(double)),
            Qt::DirectConnection);
    m_pLoopOutButton->set(0);

    m_pLoopExitButton = new ControlPushButton(ConfigKey(group, "loop_exit"));
    connect(m_pLoopExitButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopExit(double)),
            Qt::DirectConnection);
    m_pLoopExitButton->set(0);

    m_pReloopExitButton = new ControlPushButton(ConfigKey(group, "reloop_exit"));
    connect(m_pReloopExitButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotReloopExit(double)),
            Qt::DirectConnection);
    m_pReloopExitButton->set(0);

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
    m_pClosestBeat = ControlObject::getControl(ConfigKey(group, "beat_closest"));
    m_pTrackSamples = ControlObject::getControl(ConfigKey(group, "track_samples"));
    m_pSlipEnabled = ControlObject::getControl(ConfigKey(group, "slip_enabled"));

    // Connect beatloop, which can flexibly handle different values.
    // Using this CO directly is meant to be used internally and by scripts,
    // or anything else that can pass in arbitrary values.
    m_pCOBeatLoop = new ControlObject(ConfigKey(group, "beatloop"), false);
    connect(m_pCOBeatLoop, SIGNAL(valueChanged(double)), this,
            SLOT(slotBeatLoop(double)), Qt::DirectConnection);


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
}

LoopingControl::~LoopingControl() {
    delete m_pLoopOutButton;
    delete m_pLoopInButton;
    delete m_pLoopExitButton;
    delete m_pReloopExitButton;
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

    delete m_pCOBeatJump;
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

void LoopingControl::slotLoopScale(double scale) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || loopSamples.end == kNoTrigger) {
        return;
    }
    int loop_length = loopSamples.end - loopSamples.start;
    int old_loop_end = loopSamples.end;
    int samples = m_pTrackSamples->get();
    loop_length *= scale;

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
    if (m_bLoopingEnabled && scale < 1.0) {
        seekInsideAdjustedLoop(
                loopSamples.start, old_loop_end,
                loopSamples.start, loopSamples.end);
    }
}

void LoopingControl::slotLoopHalve(double v) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || 
            loopSamples.end == kNoTrigger ||
            v <= 0.0) {
        return;
    }

    // If a beatloop is active then halve should deactive the current
    // beatloop and activate the previous one.
    BeatLoopingControl* pActiveBeatLoop = m_pActiveBeatLoop;
    if (pActiveBeatLoop != nullptr) {
        int active_index = m_beatLoops.indexOf(pActiveBeatLoop);
        if (active_index - 1 >= 0) {
            if (m_bLoopingEnabled) {
                // If the current position is outside the range of the new loop,
                // take the current position and subtract the length of the new loop until
                // it fits.
                int old_loop_in = loopSamples.start;
                int old_loop_out = loopSamples.end;
                slotBeatLoopActivate(m_beatLoops[active_index - 1]);
                loopSamples = m_loopSamples.getValue();
                seekInsideAdjustedLoop(
                        old_loop_in, old_loop_out,
                        loopSamples.start, loopSamples.end);
            } else {
                // Calling scale clears the active beatloop.
                slotLoopScale(0.5);
                m_pActiveBeatLoop = m_beatLoops[active_index - 1];
            }
        }
    } else {
        slotLoopScale(0.5);
    }
}

void LoopingControl::slotLoopDouble(double v) {
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || 
            loopSamples.end == kNoTrigger ||
            v <= 0.0) {
        return;
    }

    // If a beatloop is active then double should deactive the current
    // beatloop and activate the next one.
    BeatLoopingControl* pActiveBeatLoop = m_pActiveBeatLoop;
    if (pActiveBeatLoop != NULL) {
        int active_index = m_beatLoops.indexOf(pActiveBeatLoop);
        if (active_index + 1 < m_beatLoops.size()) {
            if (m_bLoopingEnabled) {
                slotBeatLoopActivate(m_beatLoops[active_index + 1]);
            } else {
                // Calling scale clears the active beatloop.
                slotLoopScale(2.0);
                m_pActiveBeatLoop = m_beatLoops[active_index + 1];
            }
        }
    } else {
        slotLoopScale(2.0);
    }
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
            retval = reverse ? loopSamples.end : loopSamples.start;
        }
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

    if (m_bLoopingEnabled) {
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

    if (m_bLoopingEnabled) {
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
            loop_hint.sample = loopSamples.start;
            loop_hint.length = 0; // Let it issue the default length
            pHintList->append(loop_hint);
        }
        if (loopSamples.end >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = loopSamples.end;
            loop_hint.length = -1; // Let it issue the default (backwards) length
            pHintList->append(loop_hint);
        }
    } else {
        if (loopSamples.start >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = loopSamples.start;
            loop_hint.length = 0; // Let it issue the default length
            pHintList->append(loop_hint);
        }
    }
}

void LoopingControl::slotLoopIn(double val) {
    if (!m_pTrack || val <= 0.0) {
        return;
    }

    clearActiveBeatLoop();

    // set loop-in position
    LoopSamples loopSamples = m_loopSamples.getValue();
    double closestBeat = -1;
    int pos = m_iCurrentSample;
    if (m_pQuantizeEnabled->toBool()) {
        closestBeat = m_pClosestBeat->get();
        if (closestBeat != -1) {
            pos = static_cast<int>(floor(closestBeat));
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
        if (closestBeat != -1 && m_pBeats) {
            pos = static_cast<int>(floor(m_pBeats->findNthBeat(closestBeat, -2)));
            if (pos == -1 || (loopSamples.end - pos) < MINIMUM_AUDIBLE_LOOP_SIZE) {
                pos = loopSamples.end - MINIMUM_AUDIBLE_LOOP_SIZE;
            }
        } else {
            pos = loopSamples.end - MINIMUM_AUDIBLE_LOOP_SIZE;
        }
    }

    if (pos != -1 && !even(pos)) {
        pos--;
    }

    loopSamples.start = pos;
    m_pCOLoopStartPosition->set(loopSamples.start);

    m_loopSamples.setValue(loopSamples);
    //qDebug() << "set loop_in to " << loopSamples.start;
}

void LoopingControl::slotLoopOut(double val) {
    if (!m_pTrack || val <= 0.0) {
        return;
    }

    LoopSamples loopSamples = m_loopSamples.getValue();
    double closestBeat = -1;
    int pos = m_iCurrentSample;
    if (m_pQuantizeEnabled->toBool()) {
        closestBeat = m_pClosestBeat->get();
        if (closestBeat != -1) {
            pos = static_cast<int>(floor(closestBeat));
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
        if (closestBeat != -1 && m_pBeats) {
            pos = static_cast<int>(floor(m_pBeats->findNthBeat(closestBeat, 2)));
            if (pos == -1 || (pos - loopSamples.start) < MINIMUM_AUDIBLE_LOOP_SIZE) {
                pos = loopSamples.start + MINIMUM_AUDIBLE_LOOP_SIZE;
            }
        } else {
            pos = loopSamples.start + MINIMUM_AUDIBLE_LOOP_SIZE;
        }
    }

    if (pos != -1 && !even(pos)) {
        pos++;  // Increment to avoid shortening too-short loops
    }

    clearActiveBeatLoop();

    // set loop out position
    loopSamples.end = pos;
    m_pCOLoopEndPosition->set(loopSamples.end);
    m_loopSamples.setValue(loopSamples);

    // start looping
    if (loopSamples.start != kNoTrigger &&
            loopSamples.end != kNoTrigger) {
        setLoopingEnabled(true);
    }
    //qDebug() << "set loop_out to " << loopSamples.end;
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

void LoopingControl::slotReloopExit(double val) {
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
        //qDebug() << "reloop_exit looping off";
    } else {
        // If we're not looping, jump to the loop-in point and start looping
        LoopSamples loopSamples = m_loopSamples.getValue();
        if (loopSamples.start != kNoTrigger && loopSamples.end != kNoTrigger &&
                loopSamples.start <= loopSamples.end) {
            setLoopingEnabled(true);
        }
        //qDebug() << "reloop_exit looping on";
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
        if (dNewPlaypos < loopSamples.start || dNewPlaypos > loopSamples.end) {
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
    slotBeatLoop(pBeatLoopControl->getSize(), m_bLoopingEnabled);
}

void LoopingControl::slotBeatLoopActivateRoll(BeatLoopingControl* pBeatLoopControl) {
     if (!m_pTrack) {
         return;
     }

    // Disregard existing loops.
    m_pSlipEnabled->set(1);
    slotBeatLoop(pBeatLoopControl->getSize(), false);
    m_bLoopRollActive = true;
}

void LoopingControl::slotBeatLoopDeactivate(BeatLoopingControl* pBeatLoopControl) {
    Q_UNUSED(pBeatLoopControl);
    setLoopingEnabled(false);
}

void LoopingControl::slotBeatLoopDeactivateRoll(BeatLoopingControl* pBeatLoopControl) {
    Q_UNUSED(pBeatLoopControl);
    setLoopingEnabled(false);
    m_pSlipEnabled->set(0);
    m_bLoopRollActive = false;
}

void LoopingControl::clearActiveBeatLoop() {
    BeatLoopingControl* pOldBeatLoop = m_pActiveBeatLoop.fetchAndStoreAcquire(nullptr);
    if (pOldBeatLoop != nullptr) {
        pOldBeatLoop->deactivate();
    }
}

void LoopingControl::slotBeatLoop(double beats, bool keepStartPoint) {
    int samples = m_pTrackSamples->get();
    if (!m_pTrack || samples == 0) {
        clearActiveBeatLoop();
        return;
    }

    if (!m_pBeats) {
        clearActiveBeatLoop();
        return;
    }

    // For now we do not handle negative beatloops.
    if (beats < 0) {
        clearActiveBeatLoop();
        return;
    }

    // O(n) search, but there are only ~10-ish beatloop controls so this is
    // fine.
    for (BeatLoopingControl* pBeatLoopControl: m_beatLoops) {
        if (pBeatLoopControl->getSize() == beats) {
            pBeatLoopControl->activate();
            BeatLoopingControl* pOldBeatLoop =
                    m_pActiveBeatLoop.fetchAndStoreRelease(pBeatLoopControl);
            if (pOldBeatLoop != nullptr && pOldBeatLoop != pBeatLoopControl) {
                pOldBeatLoop->deactivate();
            }
            break;
        }
    }

    // give start and end defaults so we can detect problems
    LoopSamples newloopSamples = {kNoTrigger, kNoTrigger};
    LoopSamples loopSamples = m_loopSamples.getValue();


    // For positive numbers we start from the current position/closest beat and
    // create the loop around X beats from there.
    if (beats > 0) {
        if (keepStartPoint) {
            newloopSamples.start = loopSamples.start;
        } else {
            // loop_in is set to the previous beat if quantize is on.  The
            // closest beat might be ahead of play position which would cause a seek.
            // TODO: If in reverse, should probably choose nextBeat.
            double cur_pos = getCurrentSample();
            double prevBeat;
            double nextBeat;
            m_pBeats->findPrevNextBeats(cur_pos, &prevBeat, &nextBeat);

            if (m_pQuantizeEnabled->get() > 0.0 && prevBeat != -1) {
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


            if (!even(newloopSamples.start)) {
                newloopSamples.start--;
            }
        }

        int fullbeats = static_cast<int>(beats);
        double fracbeats = beats - static_cast<double>(fullbeats);

        // Now we need to calculate the length of the beatloop. We do this by
        // taking the current beat and the fullbeats'th beat and measuring the
        // distance between them.
        newloopSamples.end = newloopSamples.start;

        if (fullbeats > 0) {
            // Add the length between this beat and the fullbeats'th beat to the
            // loop_out position;
            // TODO: figure out how to convert this to a findPrevNext call.
            double this_beat = m_pBeats->findNthBeat(newloopSamples.start, 1);
            double nth_beat = m_pBeats->findNthBeat(newloopSamples.start, 1 + fullbeats);
            newloopSamples.end += (nth_beat - this_beat);
        }

        if (fracbeats > 0) {
            // Add the fraction of the beat following the current loop_out
            // position to loop out.
            // TODO: figure out how to convert this to a findPrevNext call.
            double loop_out_beat = m_pBeats->findNthBeat(newloopSamples.end, 1);
            double loop_out_next_beat = m_pBeats->findNthBeat(newloopSamples.end, 2);
            newloopSamples.end += (loop_out_next_beat - loop_out_beat) * fracbeats;
        }
    }

    if ((newloopSamples.start == kNoTrigger) || (newloopSamples.end == kNoTrigger))
        return;

    if (!even(newloopSamples.start)) {
        newloopSamples.start--;
    }
    if (!even(newloopSamples.end)) {
        newloopSamples.end--;
    }

    if (newloopSamples.start == newloopSamples.end) {
        if ((newloopSamples.end + 2) > samples) {
            newloopSamples.start -= 2;
        } else {
            newloopSamples.end += 2;
        }
    } else if (newloopSamples.end > samples) {
        // Do not allow beat loops to go beyond the end of the track
        newloopSamples.end = samples;
    }

    if (keepStartPoint) {
        seekInsideAdjustedLoop(loopSamples.start, loopSamples.end,
                newloopSamples.start, newloopSamples.end);
    }

    m_loopSamples.setValue(newloopSamples);
    m_pCOLoopStartPosition->set(newloopSamples.start);
    m_pCOLoopEndPosition->set(newloopSamples.end);
    setLoopingEnabled(true);
}

void LoopingControl::slotBeatJump(double beats) {
    if (!m_pTrack || !m_pBeats) {
        return;
    }

    double dPosition = getCurrentSample();
    double dBeatLength;
    if (BpmControl::getBeatContext(m_pBeats, dPosition,
                                   NULL, NULL, &dBeatLength, NULL)) {
        seekAbs(dPosition + beats * dBeatLength);
    }
}

void LoopingControl::slotLoopMove(double beats) {
    if (!m_pTrack || !m_pBeats) {
        return;
    }
    LoopSamples loopSamples = m_loopSamples.getValue();
    if (loopSamples.start == kNoTrigger || loopSamples.end == kNoTrigger) {
        return;
    }

    double dPosition = getCurrentSample();
    double dBeatLength;
    if (BpmControl::getBeatContext(m_pBeats, dPosition,
                                   NULL, NULL, &dBeatLength, NULL)) {
        int old_loop_in = loopSamples.start;
        int old_loop_out = loopSamples.end;
        int new_loop_in = old_loop_in + (beats * dBeatLength);
        int new_loop_out = old_loop_out + (beats * dBeatLength);
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

void BeatJumpControl::slotJumpBackward(double v) {
    if (v > 0) {
        emit(beatJump(-m_dBeatJumpSize));
    }
}

void BeatJumpControl::slotJumpForward(double v) {
    if (v > 0) {
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
        m_pEnabled->set(0);
        m_pLegacy->set(0);
    }
}

void BeatLoopingControl::activate() {
    if (!m_bActive) {
        m_bActive = true;
        m_pEnabled->set(1);
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
