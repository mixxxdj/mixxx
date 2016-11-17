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

SINT ReadAheadManager::getNextSamples(double dRate, CSAMPLE* pOutput,
        SINT requested_samples) {
    // TODO(XXX): Remove implicit assumption of 2 channels
    if (!even(requested_samples)) {
        qDebug() << "ERROR: Non-even requested_samples to ReadAheadManager::getNextSamples";
        requested_samples--;
    }
    bool in_reverse = dRate < 0;

    //qDebug() << "start" << start_sample << requested_samples;


    // A loop will only limit the amount we can read in one shot.
    const double loop_trigger = m_pLoopingControl->nextTrigger(
            dRate, m_currentPosition, 0, 0);
    bool loop_active = loop_trigger != kNoTrigger;
    SINT preloop_samples = 0;
    double samplesToLoopTrigger = 0.0;

    SINT samples_from_reader = requested_samples;
    if (loop_active) {
        samplesToLoopTrigger = in_reverse ?
                m_currentPosition - loop_trigger :
                loop_trigger - m_currentPosition;
        if (samplesToLoopTrigger < 0) {
            // We have already passed the loop trigger
            samples_from_reader = 0;
        } else {
            // We can only read whole frames from the reader.
            // Use ceil here, to be sure to reach the loop trigger.
            preloop_samples = SampleUtil::ceilPlayPosToFrameStart(samplesToLoopTrigger,
                    kNumChannels);
            // clamp requested samples from the caller to the loop trigger point
            samples_from_reader = math_clamp(requested_samples,
                    static_cast<SINT>(0), preloop_samples);
        }
    }

    // Sanity checks.
    if (samples_from_reader < 0) {
        qDebug() << "Need negative samples in ReadAheadManager::getNextSamples. Ignoring read";
        return 0;
    }

    SINT start_sample = SampleUtil::roundPlayPosToFrameStart(
            m_currentPosition, kNumChannels);

    SINT samples_read = m_pReader->read(
            start_sample, samples_from_reader, in_reverse, pOutput);

    if (samples_read != samples_from_reader) {
        qDebug() << "didn't get what we wanted" << samples_read << samples_from_reader;
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
        const double loop_target = m_pLoopingControl->process(
                dRate, m_currentPosition, 0, 0);

        if (loop_target != kNoTrigger) {
            m_currentPosition = loop_target;
            if (preloop_samples > 0) {
                // we are up to one frame ahead of the loop trigger
                double overshoot = preloop_samples - samplesToLoopTrigger;
                // start the loop later accordingly to be sure the loop length is as desired
                // e.g. exactly one bar.
                m_currentPosition += overshoot;

                // Example in frames;
                // loop start 1.1 loop end 3.3 loop length 2.2
                // m_currentPosition samplesToLoopTrigger preloop_samples
                // 2.0               1.3                  2
                // 1.8               1.5                  2
                // 1.6               1.7                  2
                // 1.4               1.9                  2
                // 1.2               2.1                  3
                // Average preloop_samples = 2.2
            }

            // start reading before the loop start point, to crossfade these samples
            // with the samples we need to the loop end
            int loop_read_position = SampleUtil::roundPlayPosToFrameStart(
                    m_currentPosition + (in_reverse ? preloop_samples : -preloop_samples), kNumChannels);

            int looping_samples_read = m_pReader->read(
                    loop_read_position, samples_read, in_reverse, m_pCrossFadeBuffer);

            if (looping_samples_read != samples_read) {
                qDebug() << "ERROR: Couldn't get all needed samples for crossfade.";
            }

            // do crossfade from the current buffer into the new loop beginning
            if (samples_read != 0) { // avoid division by zero
                SampleUtil::linearCrossfadeBuffers(pOutput, pOutput, m_pCrossFadeBuffer, samples_read);
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
double ReadAheadManager::getFilePlaypositionFromLog(double currentFilePlayposition,
                                                             double numConsumedSamples) {
    if (numConsumedSamples == 0) {
        return currentFilePlayposition;
    }

    if (m_readAheadLog.size() == 0) {
        // No log entries to read from.
        qDebug() << this << "No read ahead log entries to read from. Case not currently handled.";
        // TODO(rryan) log through a stats pipe eventually
        return currentFilePlayposition;
    }

    double filePlayposition = 0;
    bool shouldNotifySeek = false;
    while (m_readAheadLog.size() > 0 && numConsumedSamples > 0) {
        ReadLogEntry& entry = m_readAheadLog.first();

        // Notify EngineControls that we have taken a seek.
        // Every new entry start with a seek
        if (shouldNotifySeek) {
            m_pLoopingControl->notifySeek(entry.virtualPlaypositionStart);
            if (m_pRateControl) {
                m_pRateControl->notifySeek(entry.virtualPlaypositionStart);
            }
        }

        // Advance our idea of the current virtual playposition to this
        // ReadLogEntry's start position.
        filePlayposition = entry.advancePlayposition(&numConsumedSamples);

        if (entry.length() == 0) {
            // This entry is empty now.
            m_readAheadLog.removeFirst();
        }
        shouldNotifySeek = true;
    }

    return filePlayposition;
}
