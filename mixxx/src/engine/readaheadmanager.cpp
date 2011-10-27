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
    int start_sample = m_iCurrentPosition;
    //qDebug() << "start" << start_sample << requested_samples;
    int samples_needed = requested_samples;
    CSAMPLE* base_buffer = buffer;

    // A loop will only limit the amount we can read in one shot.

    QPair<int, double> next_loop;
    // = getSoonestTrigger(in_reverse, m_iCurrentPosition);

    // Instead of used getSoonestTrigger, instead we just use the result of the
    // first engine control.
    next_loop.first = 0;
    next_loop.second = m_sEngineControls[0]->nextTrigger(dRate,
                                                         m_iCurrentPosition,
                                                         0, 0);

    if (next_loop.second != kNoTrigger) {
        int samples_available;
        if (in_reverse) {
            samples_available = m_iCurrentPosition - next_loop.second;
        } else {
            samples_available = next_loop.second - m_iCurrentPosition;
        }
        samples_needed = math_max(0, math_min(samples_needed,
                                              samples_available));
    }

    if (in_reverse) {
        start_sample = m_iCurrentPosition - samples_needed;
        /*if (start_sample < 0) {
            samples_needed = math_max(0, samples_needed + start_sample);
            start_sample = 0;
        }*/
    }

    // Sanity checks
    //Q_ASSERT(start_sample >= 0);
    Q_ASSERT(samples_needed >= 0);

    int samples_read = m_pReader->read(start_sample, samples_needed,
                                       base_buffer);

    if (samples_read != samples_needed)
        qDebug() << "didn't get what we wanted" << samples_read << samples_needed;

    // Increment or decrement current read-ahead position
    if (in_reverse) {
        m_iCurrentPosition -= samples_read;
    } else {
        m_iCurrentPosition += samples_read;
    }

    // Activate on this trigger if necessary
    if (next_loop.second != kNoTrigger) {
        double loop_trigger = next_loop.second;
        double loop_target = m_sEngineControls[next_loop.first]->
            getTrigger(dRate,
                       m_iCurrentPosition,
                       0, 0);

        if ((in_reverse && m_iCurrentPosition <= loop_trigger) ||
            (!in_reverse && m_iCurrentPosition >= loop_trigger)) {
            m_iCurrentPosition = loop_target;
        }
    }

    // Reverse the samples in-place
    if (in_reverse) {
        // TODO(rryan) pull this into MixxxUtil or something
        CSAMPLE temp1, temp2;
        for (int j = 0; j < samples_read/2; j += 2) {
            const int endpos = samples_read-1-j-1;
            temp1 = base_buffer[j];
            temp2 = base_buffer[j+1];
            base_buffer[j] = base_buffer[endpos];
            base_buffer[j+1] = base_buffer[endpos+1];
            base_buffer[endpos] = temp1;
            base_buffer[endpos+1] = temp2;
        }
    }

    //qDebug() << "read" << m_iCurrentPosition << samples_read;
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

void ReadAheadManager::hintReader(double dRate, QList<Hint>& hintList,
                                  int iSamplesPerBuffer) {
    bool in_reverse = dRate < 0;
    Hint current_position;

    // SoundTouch can read up to 2 chunks ahead. Always keep 2 chunks ahead in
    // cache.
    int length_to_cache = 2*CachingReader::kSamplesPerChunk;

    current_position.length = length_to_cache;
    current_position.sample = in_reverse ?
            m_iCurrentPosition - length_to_cache :
            m_iCurrentPosition;

    // If we are trying to cache before the start of the track,
    // Then we don't need to cache because it's all zeros!
    if (current_position.sample < 0 &&
        current_position.sample + current_position.length < 0)
        return;

    // top priority, we need to read this data immediately
    current_position.priority = 1;
    hintList.append(current_position);
}
