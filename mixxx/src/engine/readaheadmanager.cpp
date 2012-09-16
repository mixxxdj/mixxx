// readaheadmanager.cpp
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>

#include "engine/readaheadmanager.h"
#include "sampleutil.h"

#include "mathstuff.h"
#include "engine/enginecontrol.h"
#include "cachingreader.h"

ReadAheadManager::ReadAheadManager(CachingReader* pReader) :
    m_iCurrentPosition(0),
    m_pReader(pReader),
    m_pCrossFadeBuffer(new CSAMPLE[MAX_BUFFER_LEN]) {
    // zero out crossfade buffer
    SampleUtil::applyGain(m_pCrossFadeBuffer, 0.0, MAX_BUFFER_LEN);
}

ReadAheadManager::~ReadAheadManager() {
    delete [] m_pCrossFadeBuffer;
}

int ReadAheadManager::getNextSamples(double dRate, CSAMPLE* buffer,
                                     int requested_samples) {
    if (!even(requested_samples)) {
        qDebug() << "ERROR: Non-even requested_samples to ReadAheadManager::getNextSamples";
        requested_samples--;
    }
    bool in_reverse = dRate < 0;
    int start_sample = m_iCurrentPosition;
    //qDebug() << "start" << start_sample << requested_samples;
    int samples_needed = requested_samples;
    CSAMPLE* base_buffer = buffer;

    // A loop will only limit the amount we can read in one shot.

    QPair<int, double> next_loop;
    next_loop.first = 0;
    next_loop.second = m_sEngineControls[0]->nextTrigger(dRate,
                                                         m_iCurrentPosition,
                                                         0, 0);
    int preloop_samples = 0;

    if (next_loop.second != kNoTrigger) {
        int samples_available;
        if (in_reverse) {
            samples_available = m_iCurrentPosition - next_loop.second;
        } else {
            samples_available = next_loop.second - m_iCurrentPosition;
        }
        samples_needed = math_max(0, math_min(samples_needed,
                                              samples_available));

        if (in_reverse) {
            preloop_samples = m_iCurrentPosition - next_loop.second;
        } else {
            preloop_samples = next_loop.second - m_iCurrentPosition;
        }
    }

    if (in_reverse) {
        start_sample = m_iCurrentPosition - samples_needed;
    }

    // Sanity checks.
    if (samples_needed < 0) {
        qDebug() << "Need negative samples in ReadAheadManager::getNextSamples. Ignoring read";
        return 0;
    }

    int samples_read = m_pReader->read(start_sample, samples_needed,
                                       base_buffer);

    if (samples_read != samples_needed)
        qDebug() << "didn't get what we wanted" << samples_read << samples_needed;

    // Increment or decrement current read-ahead position
    if (in_reverse) {
        addReadLogEntry(m_iCurrentPosition, m_iCurrentPosition - samples_read);
        m_iCurrentPosition -= samples_read;
    } else {
        addReadLogEntry(m_iCurrentPosition, m_iCurrentPosition + samples_read);
        m_iCurrentPosition += samples_read;
    }

    // Activate on this trigger if necessary
    if (next_loop.second != kNoTrigger) {
        double loop_trigger = next_loop.second;
        double loop_target = m_sEngineControls[next_loop.first]->
            getTrigger(dRate,
                       m_iCurrentPosition,
                       0, 0);

        if (loop_target != kNoTrigger &&
            ((in_reverse && m_iCurrentPosition <= loop_trigger) ||
            (!in_reverse && m_iCurrentPosition >= loop_trigger))) {

            m_iCurrentPosition = loop_target;

            int loop_read_position = m_iCurrentPosition;
            if (in_reverse) {
                loop_read_position += preloop_samples;
            } else {
                loop_read_position -= preloop_samples;
            }

            int looping_samples_read = m_pReader->read(
                loop_read_position, samples_read, m_pCrossFadeBuffer);

            if (looping_samples_read != samples_read) {
                qDebug() << "ERROR: Couldn't get all needed samples for crossfade.";
            }

            // do crossfade from the current buffer into the new loop beginning
            double mix_amount = 0.0;
            double mix_inc = 2.0 / static_cast<double>(samples_read);
            for (int i = 0; i < samples_read; i += 2) {
                base_buffer[i] = base_buffer[i] * (1.0 - mix_amount) + m_pCrossFadeBuffer[i] * mix_amount;
                base_buffer[i+1] = base_buffer[i+1] * (1.0 - mix_amount) + m_pCrossFadeBuffer[i+1] * mix_amount;
                mix_amount += mix_inc;
            }
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
    if (pControl == NULL) {
        return;
    }
    m_sEngineControls.append(pControl);
}

void ReadAheadManager::setNewPlaypos(int iNewPlaypos) {
    QMutexLocker locker(&m_mutex);
    m_iCurrentPosition = iNewPlaypos;
    m_readAheadLog.clear();
}

void ReadAheadManager::notifySeek(int iSeekPosition) {
    QMutexLocker locker(&m_mutex);
    m_iCurrentPosition = iSeekPosition;
    m_readAheadLog.clear();

    // TODO(XXX) notifySeek on the engine controls. EngineBuffer currently does
    // a fine job of this so it isn't really necessary but eventually I think
    // RAMAN should do this job. rryan 11/2011

    // foreach (EngineControl* pControl, m_sEngineControls) {
    //     pControl->notifySeek(iSeekPosition);
    // }
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

void ReadAheadManager::addReadLogEntry(double virtualPlaypositionStart,
                                       double virtualPlaypositionEndNonInclusive) {
    QMutexLocker locker(&m_mutex);
    ReadLogEntry newEntry(virtualPlaypositionStart,
                          virtualPlaypositionEndNonInclusive);
    if (m_readAheadLog.size() > 0) {
        ReadLogEntry& last = m_readAheadLog.last();
        if (last.merge(newEntry)) {
            return;
        }
    }
    m_readAheadLog.append(newEntry);
}

int ReadAheadManager::getEffectiveVirtualPlaypositionFromLog(double currentVirtualPlayposition,
                                                             double numConsumedSamples) {
    if (numConsumedSamples == 0) {
        return currentVirtualPlayposition;
    }

    QMutexLocker locker(&m_mutex);
    if (m_readAheadLog.size() == 0) {
        // No log entries to read from.
        qDebug() << this << "No read ahead log entries to read from. Case not currently handled.";
        // TODO(rryan) log through a stats pipe eventually
        return currentVirtualPlayposition;
    }

    double virtualPlayposition = 0;
    bool shouldNotifySeek = false;
    bool direction = true;
    while (m_readAheadLog.size() > 0 && numConsumedSamples > 0) {
        ReadLogEntry& entry = m_readAheadLog.first();
        direction = entry.direction();

        // Notify EngineControls that we have taken a seek.
        if (shouldNotifySeek) {
            foreach (EngineControl* pControl, m_sEngineControls) {
                pControl->notifySeek(entry.virtualPlaypositionStart);
            }
        }

        double consumed = entry.consume(numConsumedSamples);
        numConsumedSamples -= consumed;

        // Advance our idea of the current virtual playposition to this
        // ReadLogEntry's start position.
        virtualPlayposition = entry.virtualPlaypositionStart;

        if (entry.length() == 0) {
            // This entry is empty now.
            m_readAheadLog.removeFirst();
        }
        shouldNotifySeek = true;
    }
    int result = 0;
    if (direction) {
        result = static_cast<int>(floor(virtualPlayposition));
        if (!even(result)) {
            result--;
        }
    } else {
        result = static_cast<int>(ceil(virtualPlayposition));
        if (!even(result)) {
            result++;
        }
    }
    return result;
}
