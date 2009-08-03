// readaheadmanager.cpp
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/readaheadmanager.h"

#include "mathstuff.h"
#include "engine/enginecontrol.h"
#include "cachingreader.h"


ReadAheadManager::ReadAheadManager(CachingReader* pReader) :
    m_iCurrentPosition(0),
    m_pReader(pReader) {
}

ReadAheadManager::~ReadAheadManager() {
}

int ReadAheadManager::getNextSamples(double dRate, CSAMPLE* buffer,
                                     int requested_samples) {
    Q_ASSERT(even(requested_samples));
    bool in_reverse = dRate < 0;

    int samples_needed = requested_samples;
    int samples_read = 0;
    QPair<int, double> next_loop = getSoonestTrigger(in_reverse, m_iCurrentPosition);

    if (0 && next_loop.second != kNoTrigger) {
        // TODO(rryan) no looping for you!
    } else {
        if (in_reverse) {
            samples_read = m_pReader->read(m_iCurrentPosition-samples_needed,
                                           samples_needed,
                                           buffer);
            // TODO(rryan) reverse the buffer
            m_iCurrentPosition -= samples_read;
        } else {
            
            samples_read = m_pReader->read(m_iCurrentPosition, samples_needed, buffer);
            //qDebug() << "reading" << m_iCurrentPosition << ":" << samples_needed << " -- got" << samples_read;
            m_iCurrentPosition += samples_read;
        }
    }
    return samples_read;
}

void ReadAheadManager::addEngineControl(EngineControl* pControl) {
    Q_ASSERT(pControl);
    m_sEngineControls.append(pControl);
}

void ReadAheadManager::setNewPlaypos(int iNewPlaypos) {
    m_iCurrentPosition = iNewPlaypos;
}

void ReadAheadManager::notifySeek(int iSeekPosition) {
    m_iCurrentPosition = iSeekPosition;
}

QPair<int, double> ReadAheadManager::getSoonestTrigger(double dRate,
                                                       int iCurrentSample) {
    bool in_reverse = dRate < 0;
    double next_trigger = kNoTrigger;
    int next_trigger_index = -1;
    int i;
    for (int i = 0; i < m_sEngineControls.size(); ++i) {
        // TODO(rryan) eh.. this interface is likely to change so dont sweat the
        // last 2 parameters for now, nothing currently uses them
        double trigger = m_sEngineControls[i]->getTrigger(dRate, iCurrentSample,
                                                          0, 0);
        bool trigger_active = (trigger != kNoTrigger &&
                               ((in_reverse && trigger <= iCurrentSample) ||
                                (!in_reverse && trigger >= iCurrentSample)));
        if (trigger_active &&
            (next_trigger == kNoTrigger ||
             (in_reverse && trigger > next_trigger) ||
             (!in_reverse && trigger < next_trigger))) {
            
            next_trigger = trigger;
            next_trigger_index = i;
        }
    }
    return qMakePair(next_trigger_index, next_trigger);
}
