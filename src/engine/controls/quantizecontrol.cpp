// QuantizeControl.cpp
// Created on Sat 5, 2011
// Author: pwhelan

#include <QtDebug>

#include "control/controlobject.h"
#include "preferences/usersettings.h"
#include "control/controlpushbutton.h"
#include "engine/controls/quantizecontrol.h"
#include "engine/controls/enginecontrol.h"
#include "util/assert.h"

QuantizeControl::QuantizeControl(QString group,
                                 UserSettingsPointer pConfig)
        : EngineControl(group, pConfig) {
    // Turn quantize OFF by default. See Bug #898213
    m_pCOQuantizeEnabled = new ControlPushButton(ConfigKey(group, "quantize"), true);
    m_pCOQuantizeEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pCONextBeat = new ControlObject(ConfigKey(group, "beat_next"));
    m_pCONextBeat->set(-1);
    m_pCOPrevBeat = new ControlObject(ConfigKey(group, "beat_prev"));
    m_pCOPrevBeat->set(-1);
    m_pCOClosestBeat = new ControlObject(ConfigKey(group, "beat_closest"));
    m_pCOClosestBeat->set(-1);
}

QuantizeControl::~QuantizeControl() {
    delete m_pCOQuantizeEnabled;
    delete m_pCONextBeat;
    delete m_pCOPrevBeat;
    delete m_pCOClosestBeat;
}

void QuantizeControl::trackLoaded(TrackPointer pNewTrack) {
    if (m_pTrack) {
        disconnect(m_pTrack.get(), &Track::beatsUpdated,
                this, &QuantizeControl::slotBeatsUpdated);
    }

    if (pNewTrack) {
        m_pTrack = pNewTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.get(), &Track::beatsUpdated,
                this, &QuantizeControl::slotBeatsUpdated);
        // Initialize prev and next beat as if current position was zero.
        // If there is a cue point, the value will be updated.
        lookupBeatPositions(0.0);
        updateClosestBeat(0.0);
    } else {
        m_pTrack.reset();
        m_pBeats.clear();
        m_pCOPrevBeat->set(-1);
        m_pCONextBeat->set(-1);
        m_pCOClosestBeat->set(-1);
    }
}

void QuantizeControl::slotBeatsUpdated() {
    TrackPointer pTrack = m_pTrack;
    if (pTrack) {
        m_pBeats = pTrack->getBeats();
        double current = getSampleOfTrack().current;
        lookupBeatPositions(current);
        updateClosestBeat(current);
    }
}

void QuantizeControl::setCurrentSample(const double dCurrentSample,
                                       const double dTotalSamples,
                                       const double dTrackSampleRate) {
    if (dCurrentSample == getSampleOfTrack().current) {
        // No need to recalculate.
        return;
    }

    EngineControl::setCurrentSample(dCurrentSample, dTotalSamples, dTrackSampleRate);
    // We only need to update the prev or next if the current sample is
    // out of range of the existing beat positions or if we've been forced to
    // do so.
    // NOTE: This bypasses the epsilon calculation, but is there a way
    //       that could actually cause a problem?
    if (dCurrentSample < m_pCOPrevBeat->get() || dCurrentSample > m_pCONextBeat->get()) {
        lookupBeatPositions(dCurrentSample);
    }
    updateClosestBeat(dCurrentSample);
}

void QuantizeControl::lookupBeatPositions(double dCurrentSample) {
    BeatsPointer pBeats = m_pBeats;
    if (pBeats) {
        double prevBeat, nextBeat;
        pBeats->findPrevNextBeats(dCurrentSample, &prevBeat, &nextBeat);
        m_pCOPrevBeat->set(prevBeat);
        m_pCONextBeat->set(nextBeat);
    }
}

void QuantizeControl::updateClosestBeat(double dCurrentSample) {
    if (!m_pBeats) {
        return;
    }
    double prevBeat = m_pCOPrevBeat->get();
    double nextBeat = m_pCONextBeat->get();
    double closestBeat = m_pCOClosestBeat->get();

    // Calculate closest beat by hand since we want the beat locations themselves
    // and duplicating the work by calling the standard API would double
    // the number of mutex locks.
    if (prevBeat == -1) {
        if (nextBeat != -1) {
            m_pCOClosestBeat->set(nextBeat);
        } else {
            // Likely no beat information -- can't set closest beat value.
        }
    } else if (nextBeat == -1) {
        m_pCOClosestBeat->set(prevBeat);
    } else {
        double currentClosestBeat =
                (nextBeat - dCurrentSample > dCurrentSample - prevBeat) ?
                        prevBeat : nextBeat;
        VERIFY_OR_DEBUG_ASSERT(even(static_cast<int>(currentClosestBeat))) {
            currentClosestBeat--;
        }
        if (closestBeat != currentClosestBeat) {
            m_pCOClosestBeat->set(currentClosestBeat);
        }
    }
}
