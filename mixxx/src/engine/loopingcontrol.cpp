// loopingcontrol.cpp
// Created on Sep 23, 2008
// Author: asantoni, rryan

#include <QtDebug>
#include <QObject>

#include "controlobject.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "cachingreader.h"
#include "engine/loopingcontrol.h"
#include "engine/enginecontrol.h"
#include "mathstuff.h"

#include "trackinfoobject.h"
#include "track/beats.h"

double LoopingControl::s_dBeatSizes[] = { 0.0625, 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32, 64, };

LoopingControl::LoopingControl(const char * _group,
                               ConfigObject<ConfigValue> * _config)
        : EngineControl(_group, _config) {
    m_bLoopingEnabled = false;
    m_iLoopStartSample = kNoTrigger;
    m_iLoopEndSample = kNoTrigger;
    m_iCurrentSample = 0.;
    m_pActiveBeatLoop = NULL;

    //Create loop-in, loop-out, and reloop/exit ControlObjects
    m_pLoopInButton = new ControlPushButton(ConfigKey(_group, "loop_in"));
    connect(m_pLoopInButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopIn(double)),
            Qt::DirectConnection);
    m_pLoopInButton->set(0);

    m_pLoopOutButton = new ControlPushButton(ConfigKey(_group, "loop_out"));
    connect(m_pLoopOutButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopOut(double)),
            Qt::DirectConnection);
    m_pLoopOutButton->set(0);

    m_pReloopExitButton = new ControlPushButton(ConfigKey(_group, "reloop_exit"));
    connect(m_pReloopExitButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotReloopExit(double)),
            Qt::DirectConnection);
    m_pReloopExitButton->set(0);


    m_pCOLoopEnabled = new ControlObject(ConfigKey(_group, "loop_enabled"));
    m_pCOLoopEnabled->set(0.0f);

    m_pCOLoopStartPosition =
            new ControlObject(ConfigKey(_group, "loop_start_position"));
    m_pCOLoopStartPosition->set(kNoTrigger);
    connect(m_pCOLoopStartPosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopStartPos(double)),
            Qt::DirectConnection);

    m_pCOLoopEndPosition =
            new ControlObject(ConfigKey(_group, "loop_end_position"));
    m_pCOLoopEndPosition->set(kNoTrigger);
    connect(m_pCOLoopEndPosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopEndPos(double)),
            Qt::DirectConnection);

    m_pQuantizeEnabled = ControlObject::getControl(ConfigKey(_group, "quantize"));
    m_pNextBeat = ControlObject::getControl(ConfigKey(_group, "beat_next"));
    m_pTrackSamples = ControlObject::getControl(ConfigKey(_group,"track_samples"));

    // Connect beatloop, which can flexibly handle different values.
    // Using this CO directly is meant to be used internally and by scripts,
    // or anything else that can pass in arbitrary values.
    m_pCOBeatLoop = new ControlPushButton(ConfigKey(_group, "beatloop"));
    connect(m_pCOBeatLoop, SIGNAL(valueChanged(double)), this,
            SLOT(slotBeatLoop(double)), Qt::DirectConnection);


    // Here we create corresponding beatloop_(SIZE) CO's which all call the same
    // BeatControl, but with a set value.
    for (unsigned int i = 0; i < (sizeof(s_dBeatSizes) / sizeof(s_dBeatSizes[0])); ++i) {
        BeatLoopingControl* pBeatLoop = new BeatLoopingControl(_group, s_dBeatSizes[i]);
        connect(pBeatLoop, SIGNAL(activateBeatLoop(BeatLoopingControl*)),
                this, SLOT(slotBeatLoopActivate(BeatLoopingControl*)),
                Qt::DirectConnection);
        connect(pBeatLoop, SIGNAL(deactivateBeatLoop(BeatLoopingControl*)),
                this, SLOT(slotBeatLoopDeactivate(BeatLoopingControl*)),
                Qt::DirectConnection);
        m_beatLoops.append(pBeatLoop);
    }

    m_pCOLoopScale = new ControlObject(ConfigKey(_group, "loop_scale"));
    connect(m_pCOLoopScale, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopScale(double)));
    m_pLoopHalveButton = new ControlPushButton(ConfigKey(_group, "loop_halve"));
    connect(m_pLoopHalveButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopHalve(double)));
    m_pLoopDoubleButton = new ControlPushButton(ConfigKey(_group, "loop_double"));
    connect(m_pLoopDoubleButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopDouble(double)));
}

LoopingControl::~LoopingControl() {
    delete m_pLoopOutButton;
    delete m_pLoopInButton;
    delete m_pReloopExitButton;
    delete m_pCOLoopEnabled;
    delete m_pCOLoopStartPosition;
    delete m_pCOLoopEndPosition;
    delete m_pCOLoopScale;
    delete m_pLoopHalveButton;
    delete m_pLoopDoubleButton;
    delete m_pCOBeatLoop;

    while (m_beatLoops.size() > 0) {
        BeatLoopingControl* pBeatLoop = m_beatLoops.takeLast();
        delete pBeatLoop;
    }
}

void LoopingControl::slotLoopScale(double scale) {
    int loop_length = m_iLoopEndSample - m_iLoopStartSample;
    int samples = m_pTrackSamples->get();
    loop_length *= scale;
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
}

void LoopingControl::slotLoopHalve(double v) {
    if (v > 0.0) {
        // If a beatloop is active then halve should deactive the current
        // beatloop and activate the previous one.
        if (m_pActiveBeatLoop != NULL) {
            int active_index = m_beatLoops.indexOf(m_pActiveBeatLoop);
            if (active_index - 1 >= 0) {
                slotBeatLoopActivate(m_beatLoops[active_index - 1]);
            }
        } else {
            slotLoopScale(0.5);
        }
    }
}

void LoopingControl::slotLoopDouble(double v) {
    if (v > 0.0f) {
        // If a beatloop is active then double should deactive the current
        // beatloop and activate the next one.
        if (m_pActiveBeatLoop != NULL) {
            int active_index = m_beatLoops.indexOf(m_pActiveBeatLoop);
            if (active_index + 1 < m_beatLoops.size()) {
                slotBeatLoopActivate(m_beatLoops[active_index + 1]);
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
    m_iCurrentSample = currentSample;
    if (!even(m_iCurrentSample))
        m_iCurrentSample--;

    bool reverse = dRate < 0;

    double retval = kNoTrigger;
    if(m_bLoopingEnabled &&
       m_iLoopStartSample != kNoTrigger &&
       m_iLoopEndSample != kNoTrigger) {
        bool outsideLoop = (currentSample > m_iLoopEndSample ||
                            currentSample < m_iLoopStartSample);
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
    bool bReverse = dRate < 0;

    if(m_bLoopingEnabled) {
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
    bool bReverse = dRate < 0;

    if(m_bLoopingEnabled) {
        if (bReverse)
            return m_iLoopEndSample;
        else
            return m_iLoopStartSample;
    }
    return kNoTrigger;
}

void LoopingControl::hintReader(QList<Hint>& hintList) {
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
            hintList.append(loop_hint);
        }
        if (m_iLoopEndSample >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = m_iLoopEndSample;
            loop_hint.length = -1; // Let it issue the default (backwards) length
            hintList.append(loop_hint);
        }
    } else {
        if (m_iLoopStartSample >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = m_iLoopStartSample;
            loop_hint.length = 0; // Let it issue the default length
            hintList.append(loop_hint);
        }
    }
}

void LoopingControl::slotLoopIn(double val) {
    if (val) {
        clearActiveBeatLoop();

        // set loop in position
        m_iLoopStartSample = m_pQuantizeEnabled->get() ? m_pNextBeat->get() : m_iCurrentSample;
        m_pCOLoopStartPosition->set(m_iLoopStartSample);

        // Reset the loop out position if it is before the loop in so that loops
        // cannot be inverted.
        if (m_iLoopEndSample != -1 &&
            m_iLoopEndSample < m_iLoopStartSample) {
            m_iLoopEndSample = -1;
            m_pCOLoopEndPosition->set(kNoTrigger);
        }
    }
}

void LoopingControl::slotLoopOut(double val) {
    if (val) {
        int pos = m_pQuantizeEnabled->get() ? m_pNextBeat->get() : m_iCurrentSample;
        // If the user is trying to set a loop-out before the loop in or without
        // having a loop-in, then ignore it.
        if (m_iLoopStartSample == -1 || pos < m_iLoopStartSample) {
            return;
        }

        clearActiveBeatLoop();

        //set loop out position and start looping
        m_iLoopEndSample = pos;
        m_pCOLoopEndPosition->set(m_iLoopEndSample);

        if (m_iLoopStartSample != -1 &&
            m_iLoopEndSample != -1) {
            setLoopingEnabled(true);
        }
        //qDebug() << "set loop_out to " << m_iLoopStartSample;
    }
}

void LoopingControl::slotReloopExit(double val) {
    if (val) {
        // If we're looping, stop looping
        if (m_bLoopingEnabled) {
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
    int newpos = pos;
    if (newpos >= 0 && !even(newpos)) {
        newpos--;
    }

    clearActiveBeatLoop();

    if (pos == -1.0f) {
        setLoopingEnabled(false);
    }

    m_iLoopStartSample = newpos;

    if (m_iLoopEndSample != -1 &&
        m_iLoopEndSample < m_iLoopStartSample) {
        m_iLoopEndSample = -1;
        m_pCOLoopEndPosition->set(kNoTrigger);
        setLoopingEnabled(false);
    }
}

void LoopingControl::slotLoopEndPos(double pos) {
    int newpos = pos;

    if (newpos >= 0 && !even(newpos)) {
        newpos--;
    }

    // Reject if the loop-in is not set, or if the new position is before the
    // start point (but not -1).
    if (m_iLoopStartSample == -1 ||
        (newpos >= 0 && newpos < m_iLoopStartSample)) {
        return;
    }

    clearActiveBeatLoop();

    if (pos == -1.0f) {
        setLoopingEnabled(false);
    }
    m_iLoopEndSample = newpos;
}

void LoopingControl::notifySeek(double dNewPlaypos) {
    if (m_bLoopingEnabled) {
        Q_ASSERT(m_iLoopStartSample != -1);
        Q_ASSERT(m_iLoopEndSample != -1);
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
    bool beatLoopAlreadyActive = m_pActiveBeatLoop != NULL;
    if (beatLoopAlreadyActive && m_pActiveBeatLoop != pBeatLoopControl) {
        m_pActiveBeatLoop->deactivate();
    }
    m_pActiveBeatLoop = pBeatLoopControl;
    m_pActiveBeatLoop->activate();

    // Maintain the current start point if there is an active beat loop and we
    // are currently looping
    slotBeatLoop(pBeatLoopControl->getSize(), beatLoopAlreadyActive && m_bLoopingEnabled);
}

void LoopingControl::slotBeatLoopDeactivate(BeatLoopingControl* pBeatLoopControl) {
    slotReloopExit(1);
}

void LoopingControl::clearActiveBeatLoop() {
    if (m_pActiveBeatLoop != NULL) {
        m_pActiveBeatLoop->deactivate();
        m_pActiveBeatLoop = NULL;
    }
}

void LoopingControl::slotBeatLoop(double beats, bool keepStartPoint)
{
    // give loop_in and loop_out defaults so we can detect problems
    int loop_in = -1;
    int loop_out = -1;
    int samples = m_pTrackSamples->get();

    if ( !m_pBeats ) {
        return;
    }

    // For now we do not handle negative beatloops.
    if ( beats < 0 )
        return;

    // For positive numbers we start from the beat before us and create the loop
    // around X beats from there.
    if ( beats > 0 )
    {
        int fullbeats = (int)floorf(beats);
        double fracbeats = beats - (double)fullbeats;

        // loop_in is set to the previous beat. If we are sitting exactly on a
        // beat then loop_in is that beat.
        if (keepStartPoint) {
            loop_in = m_iLoopStartSample;
        } else {
            loop_in = m_pBeats->findNthBeat(m_iCurrentSample, -1);
        }

        int loop_out_next = -1;

        if (fullbeats > 0) {
            // 1 would have just returned loop_in, so we give 1+fullbeats to get
            // the fullbeats'th next beat.
            loop_out = m_pBeats->findNthBeat(loop_in, 1+fullbeats);
            // And we want the beat after loop_out, 1 would just return
            // loop_out.
            loop_out_next = m_pBeats->findNthBeat(loop_out, 2);
        } else {
            loop_out = loop_in;
            // 1 would have just returned loop_in, so give 2 to get the beat
            // following loop_in
            loop_out_next = m_pBeats->findNthBeat(loop_in, 2);
        }

        // qDebug() << m_iCurrentSample << "beats:" << beats << "fullbeats:" << fullbeats
        //          << "fracbeats:" << fracbeats << "loop_in" << loop_in << "loop_out_next:" << loop_out_next
        //          << "loop_out:" << loop_out;

        loop_out += (loop_out_next - loop_out) * fracbeats;
    }

    if ((loop_in == -1) || ( loop_out == -1))
        return;

    if (!even(loop_in))
        loop_in--;
    if (!even(loop_out))
        loop_out--;

    if ( loop_in == loop_out ) {
        if ((loop_out+2) > samples)
            loop_in -= 2;
        else
            loop_out += 2;
    }
    // Do not allow beat loops to go beyond the end of the track
    else if (loop_out > samples)
        loop_out = samples;

    m_iLoopStartSample = loop_in;
    m_pCOLoopStartPosition->set(loop_in);
    m_iLoopEndSample = loop_out;
    m_pCOLoopEndPosition->set(loop_out);
    setLoopingEnabled(true);
}

BeatLoopingControl::BeatLoopingControl(const char* pGroup, double size)
        : m_dBeatLoopSize(size) {
    m_pPBActivateBeatLoop = new ControlPushButton(keyForControl(pGroup, "beatloop", size));
    m_pPBActivateBeatLoop->setToggleButton(true);
    m_pPBActivateBeatLoop->setStates(2);
    connect(m_pPBActivateBeatLoop, SIGNAL(valueChanged(double)),
            this, SLOT(slotBeatLoopActivate(double)),
            Qt::DirectConnection);
}

BeatLoopingControl::~BeatLoopingControl() {
    delete m_pPBActivateBeatLoop;
}

void BeatLoopingControl::deactivate() {
    m_pPBActivateBeatLoop->set(0);
}

void BeatLoopingControl::activate() {
    m_pPBActivateBeatLoop->set(1);
}

void BeatLoopingControl::slotBeatLoopActivate(double v) {
    if (v > 0) {
        emit(activateBeatLoop(this));
    } else {
        emit(deactivateBeatLoop(this));
    }
}

ConfigKey BeatLoopingControl::keyForControl(const char* pGroup, QString ctrlName, double num) {
    ConfigKey key;
    key.group = pGroup;
    key.item = QString("%1_%2").arg(ctrlName).arg(num);
    return key;
}

