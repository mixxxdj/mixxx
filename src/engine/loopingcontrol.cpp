// loopingcontrol.cpp
// Created on Sep 23, 2008
// Author: asantoni, rryan

#include <QtDebug>

#include "controlobject.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "cachingreader.h"
#include "engine/loopingcontrol.h"
#include "engine/bpmcontrol.h"
#include "engine/enginecontrol.h"
#include "util/math.h"

#include "trackinfoobject.h"
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
                               ConfigObject<ConfigValue>* _config)
        : EngineControl(group, _config) {
    m_bLoopingEnabled = false;
    m_bLoopRollActive = false;
    m_iLoopStartSample = kNoTrigger;
    m_iLoopEndSample = kNoTrigger;
    m_iCurrentSample = 0.;
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
    if (m_iLoopStartSample == kNoTrigger || m_iLoopEndSample == kNoTrigger) {
        return;
    }
    int loop_length = m_iLoopEndSample - m_iLoopStartSample;
    int old_loop_end = m_iLoopEndSample;
    int samples = m_pTrackSamples->get();
    loop_length *= scale;

    // Abandon loops that are too short of extend beyond the end of the file.
    if (loop_length < MINIMUM_AUDIBLE_LOOP_SIZE ||
        m_iLoopStartSample + loop_length > samples) {
        return;
    }

    m_iLoopEndSample = m_iLoopStartSample + loop_length;

    if (!even(m_iLoopEndSample)) {
        m_iLoopEndSample--;
    }

    // TODO(XXX) we could be smarter about taking the active beatloop, scaling
    // it by the desired amount and trying to find another beatloop that matches
    // it, but for now we just clear the active beat loop if somebody scales.
    clearActiveBeatLoop();

    // Don't allow 0 samples loop, so one can still manipulate it
    if (m_iLoopEndSample == m_iLoopStartSample) {
        if ((m_iLoopEndSample+2) >= samples)
            m_iLoopStartSample -= 2;
        else
            m_iLoopEndSample += 2;
    }
    // Do not allow loops to go past the end of the song
    else if (m_iLoopEndSample > samples)
        m_iLoopEndSample = samples;

    // Update CO for loop end marker
    m_pCOLoopEndPosition->set(m_iLoopEndSample);

    // Reseek if the loop shrank out from under the playposition.
    if (scale < 1.0) {
        seekInsideAdjustedLoop(
                m_iLoopStartSample, old_loop_end,
                m_iLoopStartSample, m_iLoopEndSample);
    }
}

void LoopingControl::slotLoopHalve(double v) {
    if (m_iLoopStartSample == kNoTrigger || m_iLoopEndSample == kNoTrigger) {
        return;
    }
    if (v > 0.0) {
        // If a beatloop is active then halve should deactive the current
        // beatloop and activate the previous one.
        if (m_pActiveBeatLoop != NULL) {
            int active_index = m_beatLoops.indexOf(m_pActiveBeatLoop);
            if (active_index - 1 >= 0) {
                if (m_bLoopingEnabled) {
                    // If the current position is outside the range of the new loop,
                    // take the current position and subtract the length of the new loop until
                    // it fits.
                    int old_loop_in = m_iLoopStartSample;
                    int old_loop_out = m_iLoopEndSample;
                    slotBeatLoopActivate(m_beatLoops[active_index - 1]);
                    seekInsideAdjustedLoop(
                            old_loop_in, old_loop_out,
                            m_iLoopStartSample, m_iLoopEndSample);
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
}

void LoopingControl::slotLoopDouble(double v) {
    if (m_iLoopStartSample == kNoTrigger || m_iLoopEndSample == kNoTrigger) {
        return;
    }
    if (v > 0.0) {
        // If a beatloop is active then double should deactive the current
        // beatloop and activate the next one.
        if (m_pActiveBeatLoop != NULL) {
            int active_index = m_beatLoops.indexOf(m_pActiveBeatLoop);
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
}

double LoopingControl::process(const double dRate,
                               const double currentSample,
                               const double totalSamples,
                               const int iBufferSize) {
    Q_UNUSED(totalSamples);
    Q_UNUSED(iBufferSize);
    m_iCurrentSample = currentSample;
    if (!even(m_iCurrentSample))
        m_iCurrentSample--;

    bool reverse = dRate < 0;

    double retval = kNoTrigger;
    if (m_bLoopingEnabled && m_iLoopStartSample != kNoTrigger &&
            m_iLoopEndSample != kNoTrigger) {
        bool outsideLoop = currentSample >= m_iLoopEndSample ||
                           currentSample <= m_iLoopStartSample;
        if (outsideLoop) {
            retval = reverse ? m_iLoopEndSample : m_iLoopStartSample;
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

    if (m_bLoopingEnabled) {
        if (bReverse)
            return m_iLoopStartSample;
        else
            return m_iLoopEndSample;
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

    if (m_bLoopingEnabled) {
        if (bReverse)
            return m_iLoopEndSample;
        else
            return m_iLoopStartSample;
    }
    return kNoTrigger;
}

void LoopingControl::hintReader(QVector<Hint>* pHintList) {
    Hint loop_hint;
    // If the loop is enabled, then this is high priority because we will loop
    // sometime potentially very soon! The current audio itself is priority 1,
    // but we will issue ourselves at priority 2.
    if (m_bLoopingEnabled) {
        // If we're looping, hint the loop in and loop out, in case we reverse
        // into it. We could save information from process to tell which
        // direction we're going in, but that this is much simpler, and hints
        // aren't that bad to make anyway.
        if (m_iLoopStartSample >= 0) {
            loop_hint.priority = 2;
            loop_hint.sample = m_iLoopStartSample;
            loop_hint.length = 0; // Let it issue the default length
            pHintList->append(loop_hint);
        }
        if (m_iLoopEndSample >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = m_iLoopEndSample;
            loop_hint.length = -1; // Let it issue the default (backwards) length
            pHintList->append(loop_hint);
        }
    } else {
        if (m_iLoopStartSample >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = m_iLoopStartSample;
            loop_hint.length = 0; // Let it issue the default length
            pHintList->append(loop_hint);
        }
    }
}

void LoopingControl::slotLoopIn(double val) {
    if (!m_pTrack) {
        return;
    }
    if (val) {
        clearActiveBeatLoop();

        // set loop-in position
        int pos =
                (m_pQuantizeEnabled->get() > 0.0 && m_pClosestBeat->get() != -1) ?
                static_cast<int>(floor(m_pClosestBeat->get())) : m_iCurrentSample;

        // If we're looping and the loop-in and out points are now so close
        //  that the loop would be inaudible (which can happen easily with
        //  quantize-to-beat enabled,) set the in point to the smallest
        //  pre-defined beatloop size instead (when possible)
        if (m_bLoopingEnabled &&
            (m_iLoopEndSample - pos) < MINIMUM_AUDIBLE_LOOP_SIZE) {
            pos = m_iLoopEndSample;
            if (m_pQuantizeEnabled->get() > 0.0 && m_pBeats) {
                // 1 would have just returned loop_in, so give 2 to get the beat
                // following loop_in
                int nextbeat = m_pBeats->findNthBeat(pos, 2);
                pos -= (nextbeat - pos) * s_dBeatSizes[0];
            }
            else pos -= MINIMUM_AUDIBLE_LOOP_SIZE;
        }

        if (pos != -1 && !even(pos)) {
            pos--;
        }

        m_iLoopStartSample = pos;
        m_pCOLoopStartPosition->set(m_iLoopStartSample);

        // Reset the loop out position if it is before the loop in so that loops
        // cannot be inverted.
        if (m_iLoopEndSample != -1 &&
            m_iLoopEndSample < m_iLoopStartSample) {
            m_iLoopEndSample = -1;
            m_pCOLoopEndPosition->set(kNoTrigger);
        }
//         qDebug() << "set loop_in to " << m_iLoopStartSample;
    }
}

void LoopingControl::slotLoopOut(double val) {
    if (!m_pTrack) {
        return;
    }
    if (val) {
        int pos =
                (m_pQuantizeEnabled->get() > 0.0 && m_pClosestBeat->get() != -1) ?
                static_cast<int>(floor(m_pClosestBeat->get())) : m_iCurrentSample;

        // If the user is trying to set a loop-out before the loop in or without
        // having a loop-in, then ignore it.
        if (m_iLoopStartSample == kNoTrigger || pos < m_iLoopStartSample) {
            return;
        }

        // If the loop-in and out points are set so close that the loop would be
        //  inaudible (which can happen easily with quantize-to-beat enabled,)
        //  use the smallest pre-defined beatloop instead (when possible)
        if (pos - m_iLoopStartSample < MINIMUM_AUDIBLE_LOOP_SIZE) {
            pos = m_iLoopStartSample;
            if (m_pQuantizeEnabled->get() > 0.0 && m_pBeats) {
                // 1 would have just returned loop_in, so give 2 to get the beat
                // following loop_in
                int nextbeat = m_pBeats->findNthBeat(m_iLoopStartSample, 2);
                pos += (nextbeat - pos) * s_dBeatSizes[0];
            } else {
                pos += MINIMUM_AUDIBLE_LOOP_SIZE;
            }
        }

        if (pos != -1 && !even(pos)) {
            pos++;  // Increment to avoid shortening too-short loops
        }

        clearActiveBeatLoop();

        //set loop out position
        m_iLoopEndSample = pos;
        m_pCOLoopEndPosition->set(m_iLoopEndSample);

        // start looping
        if (m_iLoopStartSample != -1 &&
            m_iLoopEndSample != -1) {
            setLoopingEnabled(true);
        }
//         qDebug() << "set loop_out to " << m_iLoopEndSample;
    }
}

void LoopingControl::slotLoopExit(double val) {
    if (!m_pTrack) {
        return;
    }
    if (val) {
        // If we're looping, stop looping
        if (m_bLoopingEnabled) {
            setLoopingEnabled(false);
        }
    }
}

void LoopingControl::slotReloopExit(double val) {
    if (!m_pTrack) {
        return;
    }
    if (val) {
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
            if (m_iLoopStartSample != -1 && m_iLoopEndSample != -1 &&
                m_iLoopStartSample <= m_iLoopEndSample) {
                setLoopingEnabled(true);
            }
            //qDebug() << "reloop_exit looping on";
        }
    }
}

void LoopingControl::slotLoopStartPos(double pos) {
    if (!m_pTrack) {
        return;
    }

    int newpos = pos;
    if (newpos != -1 && !even(newpos)) {
        newpos--;
    }


    if (m_iLoopStartSample == newpos) {
        //nothing to do
        return;
    }

    clearActiveBeatLoop();

    if (pos == -1.0) {
        setLoopingEnabled(false);
    }

    m_iLoopStartSample = newpos;
    m_pCOLoopStartPosition->set(newpos);

    if (m_iLoopEndSample != -1 &&
        m_iLoopEndSample < m_iLoopStartSample) {
        m_iLoopEndSample = -1;
        m_pCOLoopEndPosition->set(kNoTrigger);
        setLoopingEnabled(false);
    }
}

void LoopingControl::slotLoopEndPos(double pos) {
    if (!m_pTrack) {
        return;
    }

    int newpos = pos;
    if (newpos != -1 && !even(newpos)) {
        newpos--;
    }

    if (m_iLoopEndSample == newpos) {
        //nothing to do
        return;
    }

    // Reject if the loop-in is not set, or if the new position is before the
    // start point (but not -1).
    if (m_iLoopStartSample == -1 ||
        (newpos != -1 && newpos < m_iLoopStartSample)) {
        m_pCOLoopEndPosition->set(m_iLoopEndSample);
        return;
    }

    clearActiveBeatLoop();

    if (pos == -1.0) {
        setLoopingEnabled(false);
    }
    m_iLoopEndSample = newpos;
    m_pCOLoopEndPosition->set(newpos);
}

void LoopingControl::notifySeek(double dNewPlaypos) {
    if (m_bLoopingEnabled) {
        if (dNewPlaypos < m_iLoopStartSample || dNewPlaypos > m_iLoopEndSample) {
            setLoopingEnabled(false);
        }
    }
}

void LoopingControl::setLoopingEnabled(bool enabled) {
    m_bLoopingEnabled = enabled;
    m_pCOLoopEnabled->set(enabled);
    if (m_pActiveBeatLoop != NULL) {
        if (enabled) {
            m_pActiveBeatLoop->activate();
        } else {
            m_pActiveBeatLoop->deactivate();
        }
    }
}

void LoopingControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    clearActiveBeatLoop();

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                this, SLOT(slotUpdatedTrackBeats()));
    }
}

void LoopingControl::trackUnloaded(TrackPointer pTrack) {
    Q_UNUSED(pTrack);
    if (m_pTrack) {
        disconnect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                   this, SLOT(slotUpdatedTrackBeats()));
    }
    m_pTrack.clear();
    m_pBeats.clear();
    clearActiveBeatLoop();
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
    if (m_pActiveBeatLoop != NULL) {
        m_pActiveBeatLoop->deactivate();
        m_pActiveBeatLoop = NULL;
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
    foreach (BeatLoopingControl* pBeatLoopControl, m_beatLoops) {
        if (pBeatLoopControl->getSize() == beats) {
            if (m_pActiveBeatLoop != pBeatLoopControl) {
                if (m_pActiveBeatLoop) {
                    m_pActiveBeatLoop->deactivate();
                }
                m_pActiveBeatLoop = pBeatLoopControl;
            }
            pBeatLoopControl->activate();
            break;
        }
    }

    // give loop_in and loop_out defaults so we can detect problems
    int loop_in = -1;
    int loop_out = -1;

    // For positive numbers we start from the current position/closest beat and
    // create the loop around X beats from there.
    if (beats > 0) {
        if (keepStartPoint) {
            loop_in = m_iLoopStartSample;
        } else {
            // loop_in is set to the previous beat if quantize is on.  The
            // closest beat might be ahead of play position which would cause a seek.
            // TODO: If in reverse, should probably choose nextBeat.
            double cur_pos = getCurrentSample();
            double prevBeat = floor(m_pBeats->findPrevBeat(cur_pos));

            if (m_pQuantizeEnabled->get() > 0.0 && prevBeat != -1) {
                if (beats >= 1.0) {
                    loop_in = prevBeat;
                } else {
                    // In case of beat length less then 1 beat:
                    // (| - beats, ^ - current track's position):
                    //
                    // ...|...................^........|...
                    //
                    // If we press 1/2 beatloop we want loop from 50% to 100%,
                    // If I press 1/4 beatloop, we want loop from 50% to 75% etc
                    double nextBeat = floor(m_pBeats->findNextBeat(cur_pos));
                    double beat_len = nextBeat - prevBeat;
                    double loops_per_beat = 1.0 / beats;
                    double beat_pos = cur_pos - prevBeat;
                    int beat_frac =
                            static_cast<int>(floor((beat_pos / beat_len) *
                                                   loops_per_beat));
                    loop_in = prevBeat + beat_len / loops_per_beat * beat_frac;
                }

            } else {
                loop_in = floor(cur_pos);
            }


            if (!even(loop_in)) {
                loop_in--;
            }
        }

        int fullbeats = static_cast<int>(beats);
        double fracbeats = beats - static_cast<double>(fullbeats);

        // Now we need to calculate the length of the beatloop. We do this by
        // taking the current beat and the fullbeats'th beat and measuring the
        // distance between them.
        loop_out = loop_in;

        if (fullbeats > 0) {
            // Add the length between this beat and the fullbeats'th beat to the
            // loop_out position;
            double this_beat = m_pBeats->findNthBeat(loop_in, 1);
            double nth_beat = m_pBeats->findNthBeat(loop_in, 1 + fullbeats);
            loop_out += (nth_beat - this_beat);
        }

        if (fracbeats > 0) {
            // Add the fraction of the beat following the current loop_out
            // position to loop out.
            double loop_out_beat = m_pBeats->findNthBeat(loop_out, 1);
            double loop_out_next_beat = m_pBeats->findNthBeat(loop_out, 2);
            loop_out += (loop_out_next_beat - loop_out_beat) * fracbeats;
        }
    }

    if ((loop_in == -1) || (loop_out == -1))
        return;

    if (!even(loop_in))
        loop_in--;
    if (!even(loop_out))
        loop_out--;

    if (loop_in == loop_out) {
        if ((loop_out+2) > samples) {
            loop_in -= 2;
        } else {
            loop_out += 2;
        }
    } else if (loop_out > samples) {
        // Do not allow beat loops to go beyond the end of the track
        loop_out = samples;
    }

    m_iLoopStartSample = loop_in;
    m_pCOLoopStartPosition->set(loop_in);
    m_iLoopEndSample = loop_out;
    m_pCOLoopEndPosition->set(loop_out);
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
    if (m_iLoopStartSample == kNoTrigger || m_iLoopEndSample == kNoTrigger) {
        return;
    }

    double dPosition = getCurrentSample();
    double dBeatLength;
    if (BpmControl::getBeatContext(m_pBeats, dPosition,
                                   NULL, NULL, &dBeatLength, NULL)) {
        int old_loop_in = m_iLoopStartSample;
        int old_loop_out = m_iLoopEndSample;
        int new_loop_in = m_iLoopStartSample + (beats * dBeatLength);
        int new_loop_out = m_iLoopEndSample + (beats * dBeatLength);
        // Should we reject any shift that goes out of bounds?

        m_iLoopStartSample = new_loop_in;
        if (m_pActiveBeatLoop) {
            // Ugly hack -- slotBeatLoop takes "true" to mean "keep starting
            // point".  It gets that in-point from m_iLoopStartSample,
            // which we just changed so that the loop actually shifts.
            slotBeatLoop(m_pActiveBeatLoop->getSize(), true);
        } else {
            m_pCOLoopStartPosition->set(new_loop_in);
            m_iLoopEndSample = new_loop_out;
            m_pCOLoopEndPosition->set(new_loop_out);
        }
        seekInsideAdjustedLoop(old_loop_in, old_loop_out,
                               new_loop_in, new_loop_out);
    }
}

void LoopingControl::seekInsideAdjustedLoop(int old_loop_in, int old_loop_out,
                                            int new_loop_in, int new_loop_out) {
    if (m_iCurrentSample >= new_loop_in && m_iCurrentSample <= new_loop_out) {
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

    int adjusted_position = m_iCurrentSample;
    while (adjusted_position > new_loop_out) {
        adjusted_position -= new_loop_size;
        if (adjusted_position < new_loop_in) {
            // I'm not even sure this is possible.  The new loop would have to be bigger than the
            // old loop, and the playhead was somehow outside the old loop.
            qWarning() << "SHOULDN'T HAPPEN: seekInsideAdjustedLoop couldn't find a new position --"
                       << " seeking to in point";
            adjusted_position = new_loop_in;
        }
    }
    while (adjusted_position < new_loop_in) {
        adjusted_position += new_loop_size;
        if (adjusted_position > new_loop_out) {
            qWarning() << "SHOULDN'T HAPPEN: seekInsideAdjustedLoop couldn't find a new position --"
                       << " seeking to in point";
            adjusted_position = new_loop_in;
        }
    }
    if (adjusted_position != m_iCurrentSample) {
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
