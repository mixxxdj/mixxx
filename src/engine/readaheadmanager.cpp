// readaheadmanager.cpp
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/readaheadmanager.h"

#include "engine/cachingreader.h"
#include "engine/loopingcontrol.h"
#include "engine/ratecontrol.h"
#include "util/defs.h"
#include "util/math.h"
#include "util/sample.h"

static const int kNumChannels = 2;

ReadAheadManager::ReadAheadManager()
        : m_pLoopingControl(NULL),
          m_pRateControl(NULL),
          m_currentPosition(0),
          m_pReader(NULL),
          m_pCrossFadeBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    // For testing only: ReadAheadManagerMock
}

ReadAheadManager::ReadAheadManager(CachingReader* pReader,
                                   LoopingControl* pLoopingControl)
        : m_pLoopingControl(pLoopingControl),
          m_pRateControl(NULL),
          m_currentPosition(0),
          m_pReader(pReader),
          m_pCrossFadeBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    DEBUG_ASSERT(m_pLoopingControl != NULL);
    DEBUG_ASSERT(m_pReader != NULL);
}

ReadAheadManager::~ReadAheadManager() {
    SampleUtil::free(m_pCrossFadeBuffer);
}

SINT ReadAheadManager::getNextSamples(double dRate, CSAMPLE* buffer,
        SINT requested_samples) {
    // TODO(XXX): Remove implicit assumption of 2 channels
    if (!even(requested_samples)) {
        qDebug() << "ERROR: Non-even requested_samples to ReadAheadManager::getNextSamples";
        requested_samples--;
    }
    bool in_reverse = dRate < 0;

    //qDebug() << "start" << start_sample << requested_samples;
    SINT samples_needed = requested_samples;
    CSAMPLE* base_buffer = buffer;

    // A loop will only limit the amount we can read in one shot.

    const double loop_trigger = m_pLoopingControl->nextTrigger(
            dRate, m_currentPosition, 0, 0);
    bool loop_active = loop_trigger != kNoTrigger;
    SINT preloop_samples = 0;

    if (loop_active) {
        double samplesToLoopTrigger = in_reverse ?
                m_currentPosition - loop_trigger :
                loop_trigger - m_currentPosition;
        if (samplesToLoopTrigger < 0) {
            samples_needed = 0;
        } else {
            // TODO() check for foor or ceil
            preloop_samples = SampleUtil::roundPlayPosToFrameStart(samplesToLoopTrigger,
                    kNumChannels);
            samples_needed = math_clamp(samples_needed,
                    static_cast<SINT>(0), preloop_samples);
        }
    }

    // TODO(DSC) we probably need to floor and ceil here?
    int start_sample;
    if (in_reverse) {
        start_sample = SampleUtil::roundPlayPosToFrameStart(
                m_currentPosition, kNumChannels) - samples_needed;
    } else {
        start_sample = SampleUtil::roundPlayPosToFrameStart(
                m_currentPosition, kNumChannels);
    }

    // Sanity checks.
    if (samples_needed < 0) {
        qDebug() << "Need negative samples in ReadAheadManager::getNextSamples. Ignoring read";
        return 0;
    }

    SINT samples_read = m_pReader->read(start_sample, in_reverse, samples_needed,
                                       base_buffer);

    if (samples_read != samples_needed) {
        qDebug() << "didn't get what we wanted" << samples_read << samples_needed;
    }

    // Increment or decrement current read-ahead position
    // Mixing int and double here is desired, because the fractional frame should
    // be resist
    if (in_reverse) {
        addReadLogEntry(m_currentPosition, m_currentPosition - samples_read);
        m_currentPosition -= samples_read;
    } else {
        addReadLogEntry(m_currentPosition, m_currentPosition + samples_read);
        m_currentPosition += samples_read;
    }

    // Activate on this trigger if necessary
    if (loop_active) {
        // LoopingControl makes the decision about whether we should loop or
        // not.
        const double loop_target = m_pLoopingControl->
                process(dRate, m_currentPosition, 0, 0);

        if (loop_target != kNoTrigger) {
            m_currentPosition = loop_target;

            // start reading before the loop start point, to crossfade these samples
            // with the samples we need to the loop end
            int loop_read_position = m_currentPosition +
                    (in_reverse ? preloop_samples : -preloop_samples);

            int looping_samples_read = m_pReader->read(
                    loop_read_position, in_reverse, samples_read, m_pCrossFadeBuffer);

            if (looping_samples_read != samples_read) {
                qDebug() << "ERROR: Couldn't get all needed samples for crossfade.";
            }

            // do crossfade from the current buffer into the new loop beginning
            if (samples_read != 0) { // avoid division by zero
                SampleUtil::linearCrossfadeBuffers(base_buffer, base_buffer, m_pCrossFadeBuffer, samples_read);
            }
        }
    }

    //qDebug() << "read" << m_currentPosition << samples_read;
    return samples_read;
}

void ReadAheadManager::addRateControl(RateControl* pRateControl) {
    m_pRateControl = pRateControl;
}

// Not thread-save, call from engine thread only
void ReadAheadManager::notifySeek(double seekPosition) {
    m_currentPosition = seekPosition;
    m_readAheadLog.clear();

    // TODO(XXX) notifySeek on the engine controls. EngineBuffer currently does
    // a fine job of this so it isn't really necessary but eventually I think
    // RAMAN should do this job. rryan 11/2011

    // foreach (EngineControl* pControl, m_sEngineControls) {
    //     pControl->notifySeek(iSeekPosition);
    // }
}

void ReadAheadManager::hintReader(double dRate, HintVector* pHintList) {
    bool in_reverse = dRate < 0;
    Hint current_position;

    // SoundTouch can read up to 2 chunks ahead. Always keep 2 chunks ahead in
    // cache.
    SINT length_to_cache = 2 * CachingReaderChunk::kSamples;

    // this called after the precious chunk was consumed
    // TODO(DSC) do wee need the floor frame?
    int sample = SampleUtil::roundPlayPosToFrameStart(
            m_currentPosition, kNumChannels);
    current_position.length = length_to_cache;
    current_position.sample = in_reverse ?
            sample - length_to_cache :
            sample;

    // If we are trying to cache before the start of the track,
    // Then we don't need to cache because it's all zeros!
    if (current_position.sample < 0 &&
        current_position.sample + current_position.length < 0)
        return;

    // top priority, we need to read this data immediately
    current_position.priority = 1;
    pHintList->append(current_position);
}

// Not thread-save, call from engine thread only
void ReadAheadManager::addReadLogEntry(double virtualPlaypositionStart,
                                       double virtualPlaypositionEndNonInclusive) {
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

// Not thread-save, call from engine thread only
SINT ReadAheadManager::getEffectiveVirtualPlaypositionFromLog(double currentVirtualPlayposition,
                                                             double numConsumedSamples) {
    if (numConsumedSamples == 0) {
        return currentVirtualPlayposition;
    }

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
            m_pLoopingControl->notifySeek(entry.virtualPlaypositionStart);
            if (m_pRateControl) {
                m_pRateControl->notifySeek(entry.virtualPlaypositionStart);
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
    SINT result = 0;
    if (direction) {
        result = static_cast<SINT>(floor(virtualPlayposition));
        // TODO(XXX): Remove implicit assumption of 2 channels
        if (!even(result)) {
            result--;
        }
    } else {
        result = static_cast<SINT>(ceil(virtualPlayposition));
        // TODO(XXX): Remove implicit assumption of 2 channels
        if (!even(result)) {
            result++;
        }
    }
    return result;
}
