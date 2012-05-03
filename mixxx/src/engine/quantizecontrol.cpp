// QuantizeControl.cpp
// Created on Sat 5, 2011
// Author: pwhelan

#include <QtDebug>
#include <QObject>

#include "controlobject.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "cachingreader.h"
#include "engine/quantizecontrol.h"
#include "engine/enginecontrol.h"
#include "mathstuff.h"

QuantizeControl::QuantizeControl(const char* pGroup,
                                 ConfigObject<ConfigValue>* pConfig)
        : EngineControl(pGroup, pConfig) {
    // Turn quantize OFF by default. See Bug #898213
    m_pCOQuantizeEnabled = new ControlPushButton(ConfigKey(pGroup, "quantize"));
    m_pCOQuantizeEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pCONextBeat = new ControlObject(ConfigKey(pGroup, "beat_next"));
    m_pCONextBeat->set(-1);
    m_pCOPrevBeat = new ControlObject(ConfigKey(pGroup, "beat_prev"));
    m_pCOPrevBeat->set(-1);
    m_pCOClosestBeat = new ControlObject(ConfigKey(pGroup, "beat_closest"));
    m_pCOClosestBeat->set(-1);
}

QuantizeControl::~QuantizeControl() {
    delete m_pCOQuantizeEnabled;
    delete m_pCONextBeat;
    delete m_pCOPrevBeat;
    delete m_pCOClosestBeat;
}

void QuantizeControl::trackLoaded(TrackPointer pTrack) {
    if (m_pTrack) {
        trackUnloaded(m_pTrack);
    }

    if (pTrack) {
        m_pTrack = pTrack;
        m_pBeats = m_pTrack->getBeats();
        connect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                this, SLOT(slotBeatsUpdated()));
    }
}

void QuantizeControl::trackUnloaded(TrackPointer pTrack) {
    if (m_pTrack) {
        disconnect(m_pTrack.data(), SIGNAL(beatsUpdated()),
                   this, SLOT(slotBeatsUpdated()));
    }
    m_pTrack.clear();
    m_pBeats.clear();
    m_pCOPrevBeat->set(-1);
    m_pCONextBeat->set(-1);
    m_pCOClosestBeat->set(-1);
}

void QuantizeControl::slotBeatsUpdated() {
    if (m_pTrack) {
        m_pBeats = m_pTrack->getBeats();
    }
}

double QuantizeControl::process(const double dRate,
                                const double currentSample,
                                const double totalSamples,
                                const int iBufferSize) {
    int iCurrentSample = currentSample;
    if (!even(iCurrentSample)) {
        iCurrentSample--;
    }

    if (!m_pBeats) {
        return kNoTrigger;
    }

    double prevBeat = m_pCOPrevBeat->get();
    double nextBeat = m_pCONextBeat->get();
    double closestBeat = m_pCOClosestBeat->get();
    double currentClosestBeat =
            floorf(m_pBeats->findClosestBeat(iCurrentSample));

    if (closestBeat != currentClosestBeat) {
        if (!even(currentClosestBeat)) {
            currentClosestBeat--;
        }
        m_pCOClosestBeat->set(currentClosestBeat);
    }

    if (prevBeat == -1 || nextBeat == -1 ||
        currentSample >= nextBeat || currentSample <= prevBeat) {
        // TODO(XXX) are the floor and even checks necessary?
        nextBeat = floorf(m_pBeats->findNextBeat(iCurrentSample));
        prevBeat = floorf(m_pBeats->findPrevBeat(iCurrentSample));

        if (!even(nextBeat))
            nextBeat--;
        if (!even(prevBeat))
            prevBeat--;

        m_pCONextBeat->set(nextBeat);
        m_pCOPrevBeat->set(prevBeat);
    }

    return kNoTrigger;
}
