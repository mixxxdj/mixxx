// readaheadmanager.cpp
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/readaheadmanager.h"

#include "engine/cachingreader/cachingreader.h"
#include "engine/controls/loopingcontrol.h"
#include "engine/controls/ratecontrol.h"
#include "util/defs.h"
#include "util/math.h"
#include "util/sample.h"

static const int kNumChannels = 2;

ReadAheadManager::ReadAheadManager()
        : m_pLoopingControl(NULL),
          m_pRateControl(NULL),
          m_currentPosition(0),
          m_pReader(NULL),
          m_pCrossFadeBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_cacheMissHappened(false) {
    // For testing only: ReadAheadManagerMock
}

ReadAheadManager::ReadAheadManager(CachingReader* pReader,
                                   LoopingControl* pLoopingControl)
        : m_pLoopingControl(pLoopingControl),
          m_pRateControl(NULL),
          m_currentPosition(0),
          m_pReader(pReader),
          m_pCrossFadeBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_cacheMissHappened(false) {
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

    double target;
    // A loop will only limit the amount we can read in one shot.
    const double loop_trigger = m_pLoopingControl->nextTrigger(
            in_reverse, m_currentPosition, &target);

    SINT preloop_samples = 0;
    double samplesToLoopTrigger = 0.0;

    bool reachedTrigger = false;

    SINT samples_from_reader = requested_samples;
    if (loop_trigger != kNoTrigger) {
        samplesToLoopTrigger = in_reverse ?
                m_currentPosition - loop_trigger :
                loop_trigger - m_currentPosition;
        if (samplesToLoopTrigger >= 0.0) {
            // We can only read whole frames from the reader.
            // Use ceil here, to be sure to reach the loop trigger.
            preloop_samples = SampleUtil::ceilPlayPosToFrameStart(
                    samplesToLoopTrigger, kNumChannels);
            // clamp requested samples from the caller to the loop trigger point
            if (preloop_samples <= requested_samples) {
                reachedTrigger = true;
                samples_from_reader = preloop_samples;
            }
        }
    }

    // Sanity checks.
    if (samples_from_reader < 0) {
        qDebug() << "Need negative samples in ReadAheadManager::getNextSamples. Ignoring read";
        return 0;
    }

    SINT start_sample = SampleUtil::roundPlayPosToFrameStart(
            m_currentPosition, kNumChannels);

    const auto readResult = m_pReader->read(
            start_sample, samples_from_reader, in_reverse, pOutput);
    if (readResult == CachingReader::ReadResult::UNAVAILABLE) {
        // Cache miss - no samples written
        SampleUtil::clear(pOutput, samples_from_reader);
        // Set the cache miss flag to decide when to apply ramping
        // after the following read attempts.
        m_cacheMissHappened = true;
    } else if (m_cacheMissHappened) {
        // Previous read was a cache miss, but now we got something back.
        // Apply ramping gain, because the last buffer has unwanted silenced
        // and new without fading are causing a pop.
        SampleUtil::applyRampingGain(pOutput, 0.0, 1.0, samples_from_reader);
        // Reset the cache miss flag, because we are now back on track.
        m_cacheMissHappened = false;
    }

    // Increment or decrement current read-ahead position
    // Mixing int and double here is desired, because the fractional frame should
    // be resist
    if (in_reverse) {
        addReadLogEntry(m_currentPosition, m_currentPosition - samples_from_reader);
        m_currentPosition -= samples_from_reader;
    } else {
        addReadLogEntry(m_currentPosition, m_currentPosition + samples_from_reader);
        m_currentPosition += samples_from_reader;
    }

    // Activate on this trigger if necessary
    if (reachedTrigger) {
        DEBUG_ASSERT(target != kNoTrigger);

        // Jump to other end of loop.
        m_currentPosition = target;
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

        const auto readResult = m_pReader->read(
                loop_read_position, samples_from_reader, in_reverse, m_pCrossFadeBuffer);
        if (readResult == CachingReader::ReadResult::UNAVAILABLE) {
            qDebug() << "ERROR: Couldn't get all needed samples for crossfade.";
            // Cache miss - no samples written
            SampleUtil::clear(m_pCrossFadeBuffer, samples_from_reader);
            // Set the cache miss flag to decide when to apply ramping
            // after the following read attempts.
            m_cacheMissHappened = true;
        }

        // do crossfade from the current buffer into the new loop beginning
        if (samples_from_reader != 0) { // avoid division by zero
            SampleUtil::linearCrossfadeBuffers(pOutput, pOutput, m_pCrossFadeBuffer, samples_from_reader);
        }
    }

    //qDebug() << "read" << m_currentPosition << samples_read;
    return samples_from_reader;
}

void ReadAheadManager::addRateControl(RateControl* pRateControl) {
    m_pRateControl = pRateControl;
}

// Not thread-save, call from engine thread only
void ReadAheadManager::notifySeek(double seekPosition) {
    m_currentPosition = seekPosition;
    m_cacheMissHappened = false;
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
    SINT frameCountToCache = 2 * CachingReaderChunk::kFrames;
    current_position.frameCount = frameCountToCache;

    // this called after the precious chunk was consumed
    if (in_reverse) {
        current_position.frame =
                static_cast<SINT>(ceil(m_currentPosition / kNumChannels)) -
                frameCountToCache;
    } else {
        current_position.frame =
                static_cast<SINT>(floor(m_currentPosition / kNumChannels));
    }

    // If we are trying to cache before the start of the track,
    // Then we don't need to cache because it's all zeros!
    if (current_position.frame < 0 &&
            current_position.frame + current_position.frameCount < 0)
    {
    	return;
    }

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
double ReadAheadManager::getFilePlaypositionFromLog(
        double currentFilePlayposition, double numConsumedSamples) {

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
        // (Not looping control)
        if (shouldNotifySeek) {
            if (m_pRateControl) {
                m_pRateControl->notifySeek(entry.virtualPlaypositionStart, false);
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
