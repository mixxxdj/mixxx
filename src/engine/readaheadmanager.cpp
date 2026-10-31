#include "engine/readaheadmanager.h"

#include "audio/frame.h"
#include "engine/cachingreader/cachingreader.h"
#include "engine/controls/cuecontrol.h"
#include "engine/controls/loopingcontrol.h"
#include "engine/controls/ratecontrol.h"
#include "util/defs.h"
#include "util/sample.h"

ReadAheadManager::ReadAheadManager()
        : m_pLoopingControl(nullptr),
          m_pCueControl(nullptr),
          m_pRateControl(nullptr),
          m_currentPosition(0),
          m_pReader(nullptr),
          m_pCrossFadeBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_cacheMissCount(0),
          m_cacheMissExpected(false) {
    // For testing only: ReadAheadManagerMock
}

ReadAheadManager::ReadAheadManager(CachingReader* pReader,
        LoopingControl* pLoopingControl,
        CueControl* pCueControl)
        : m_pLoopingControl(pLoopingControl),
          m_pCueControl(pCueControl),
          m_pRateControl(nullptr),
          m_currentPosition(0),
          m_pReader(pReader),
          m_pCrossFadeBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_cacheMissCount(0),
          m_cacheMissExpected(false) {
    DEBUG_ASSERT(m_pLoopingControl != nullptr);
    DEBUG_ASSERT(m_pCueControl != nullptr);
    DEBUG_ASSERT(m_pReader != nullptr);
}

ReadAheadManager::~ReadAheadManager() {
    SampleUtil::free(m_pCrossFadeBuffer);
}

SINT ReadAheadManager::getNextSamples(double dRate,
        CSAMPLE* pOutput,
        SINT requested_samples,
        mixxx::audio::ChannelCount channelCount) {
    // qDebug() << "getNextSamples:" << m_currentPosition << requested_samples;

    int modSamples = requested_samples % channelCount;
    if (modSamples != 0) {
        qDebug() << "ERROR: Non-aligned requested_samples to ReadAheadManager::getNextSamples";
        requested_samples -= modSamples;
    }
    bool in_reverse = dRate < 0;

    mixxx::audio::FramePos targetPosition;
    // A loop (beat loop or track on repeat) will only limit the amount we
    // can read in one shot.
    const mixxx::audio::FramePos loopTriggerPosition =
            m_pLoopingControl->nextTrigger(in_reverse,
                    mixxx::audio::FramePos::fromSamplePosMaybeInvalid(
                            m_currentPosition, channelCount),
                    &targetPosition);
    const double loop_trigger = loopTriggerPosition.toSamplePosMaybeInvalid(channelCount);
    double target = targetPosition.toSamplePosMaybeInvalid(channelCount);

    SINT preseek_samples = 0;
    double samplesToSeekTrigger = 0.0;

    bool reachedTrigger = false;

    // By default, we are reading as many sampler as requested
    SINT samples_from_reader = requested_samples;
    if (loop_trigger != kNoTrigger) {
        samplesToSeekTrigger = in_reverse ? m_currentPosition - loop_trigger
                                          : loop_trigger - m_currentPosition;
        if (samplesToSeekTrigger >= 0.0) {
            // We can only read whole frames from the reader.
            // Use ceil here, to be sure to reach the loop trigger.
            preseek_samples = SampleUtil::ceilPlayPosToFrameStart(
                    samplesToSeekTrigger, channelCount);
            // clamp requested samples from the caller to the loop trigger point
            if (preseek_samples <= requested_samples) {
                reachedTrigger = true;
                samples_from_reader = preseek_samples;
            }
        }
    }

    mixxx::audio::FramePos jumpTargetPosition;
    // A saved jump cue will only limit the amount we can read in one shot.
    const mixxx::audio::FramePos jumpTriggerPosition =
            m_pCueControl->nextTrigger(in_reverse,
                    mixxx::audio::FramePos::fromSamplePosMaybeInvalid(
                            m_currentPosition, channelCount),
                    &jumpTargetPosition,
                    static_cast<mixxx::audio::FrameDiff_t>(requested_samples / channelCount));
    double jump_trigger = jumpTriggerPosition.toSamplePosMaybeInvalid(channelCount);

    // If there is both a loop and saved jump that are armed, and they both
    // cancel each other (Loop from A -> B, jump from A -> B), we no-op the jump
    // to prevent an infinite silent play loop
    if (jump_trigger != kNoTrigger && loop_trigger != kNoTrigger &&
            jumpTriggerPosition == targetPosition &&
            loopTriggerPosition == jumpTargetPosition) {
        jump_trigger = kNoTrigger;
    }

    SINT prejump_samples = 0;
    double samplesToJumpTrigger = 0.0;

    if (jump_trigger != kNoTrigger) {
        samplesToJumpTrigger = in_reverse ? m_currentPosition - jump_trigger
                                          : jump_trigger - m_currentPosition;
        if (samplesToJumpTrigger >= 0.0) {
            // We can only read whole frames from the reader.
            // Use ceil here, to be sure to reach the jump trigger.
            prejump_samples = SampleUtil::ceilPlayPosToFrameStart(
                    samplesToJumpTrigger, channelCount);
            // clamp requested samples from the caller to the jump trigger point
            if (prejump_samples <= requested_samples) {
                reachedTrigger = true;
                // A loop end may be before the jump. If the jump is first, this
                // should be our new target
                if (loop_trigger == kNoTrigger || prejump_samples < preseek_samples) {
                    samples_from_reader = prejump_samples;
                    preseek_samples = prejump_samples;
                    samplesToSeekTrigger = samplesToJumpTrigger;
                    target = jumpTargetPosition.toSamplePosMaybeInvalid(channelCount);
                    targetPosition = jumpTargetPosition;
                }
            }
        }
    }

    // Sanity checks.
    VERIFY_OR_DEBUG_ASSERT(samples_from_reader >= 0) {
        qDebug() << "Need negative samples in ReadAheadManager::getNextSamples. Ignoring read";
        return 0;
    }

    SINT start_sample = SampleUtil::roundPlayPosToFrameStart(
            m_currentPosition, channelCount);

    const auto readResult = m_pReader->read(
            start_sample, samples_from_reader, in_reverse, pOutput, channelCount);
    if (readResult == CachingReader::ReadResult::UNAVAILABLE) {
        // Cache miss - no samples written
        SampleUtil::clear(pOutput, samples_from_reader);
        // Set the cache miss flag to decide when to apply ramping
        // after the following read attempts.
        m_cacheMissCount++;
    } else if (m_cacheMissCount > 0) {
        // Previous read was a cache miss, but now we got something back.
        // Apply ramping gain, because the last buffer has unwanted silence
        // and new samples without fading are causing a pop.
        SampleUtil::applyRampingGain(pOutput,
                CSAMPLE_GAIN_ZERO,
                CSAMPLE_GAIN_ONE,
                samples_from_reader);
        // Reset the cache miss flag, because we are now back on track.
        if (!m_cacheMissExpected) {
            qDebug() << "ReadAheadManager: continue after number cache misses:" << m_cacheMissCount;
        }
        m_cacheMissCount = 0;
        m_cacheMissExpected = false;
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
        if (m_pRateControl) {
            m_pRateControl->notifyWrapAround(loopTriggerPosition.isValid()
                            ? loopTriggerPosition
                            : jumpTriggerPosition,
                    targetPosition);
        }
        // TODO probably also useful for hotcue_X_indicator in CueControl::updateIndicators()

        // Jump to other end of loop or track.
        m_currentPosition = target;
        if (preseek_samples > 0) {
            // we are up to one frame ahead of the loop trigger
            double overshoot = preseek_samples - samplesToSeekTrigger;
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

        // start reading before the loop start point or the saved jump, to crossfade these samples
        // with the samples we need to the loop end
        int seek_read_position = SampleUtil::roundPlayPosToFrameStart(
                m_currentPosition +
                        (in_reverse ? preseek_samples : -preseek_samples),
                channelCount);

        int crossFadeStart = 0;
        int crossFadeSamples = samples_from_reader;
        if (seek_read_position < 0) {
            // we start in the pre-role without suitable samples for crossfading
            crossFadeStart = -seek_read_position;
            crossFadeSamples -= crossFadeStart;
        } else {
            int trackSamples = static_cast<int>(
                    m_pLoopingControl->getTrackFrame().toSamplePos(
                            channelCount));
            if (seek_read_position > trackSamples) {
                // looping in reverse overlapping post-roll without samples
                crossFadeStart = seek_read_position - trackSamples;
                crossFadeSamples -= crossFadeStart;
            }
        }

        if (crossFadeSamples > 0) {
            const auto readResult = m_pReader->read(seek_read_position +
                            (in_reverse ? crossFadeStart : -crossFadeStart),
                    crossFadeSamples,
                    in_reverse,
                    m_pCrossFadeBuffer,
                    channelCount);
            if (readResult == CachingReader::ReadResult::UNAVAILABLE) {
                qDebug() << "ERROR: Couldn't get all needed samples for crossfade.";
                // Cache miss - no samples written
                SampleUtil::clear(m_pCrossFadeBuffer, samples_from_reader);
                // Set the cache miss flag to decide when to apply ramping
                // after the following read attempts.
                m_cacheMissCount++;
            }

            // do crossfade from the current buffer into the new loop beginning
            if (samples_from_reader != 0) { // avoid division by zero
                SampleUtil::linearCrossfadeBuffersOut(
                        pOutput + SampleUtil::ceilPlayPosToFrameStart(crossFadeStart, channelCount),
                        m_pCrossFadeBuffer,
                        crossFadeSamples,
                        channelCount);
            }
        } else {
            // No samples for crossfading, ramp to zero
            SampleUtil::applyRampingGain(pOutput,
                    CSAMPLE_GAIN_ONE,
                    CSAMPLE_GAIN_ZERO,
                    samples_from_reader);
        }
    }

    // qDebug() << "read" << m_currentPosition << samples_from_reader;
    return samples_from_reader;
}

void ReadAheadManager::addRateControl(RateControl* pRateControl) {
    m_pRateControl = pRateControl;
}

// Not thread-save, call from engine thread only
void ReadAheadManager::notifySeek(double seekPosition) {
    m_currentPosition = seekPosition;
    m_cacheMissCount = 0;
    m_cacheMissExpected = true;
    m_readAheadLog.clear();
}

void ReadAheadManager::hintReader(double dRate,
        gsl::not_null<HintVector*> pHintList,
        mixxx::audio::ChannelCount channelCount) {
    bool in_reverse = dRate < 0;
    Hint current_position;

    // SoundTouch can read up to 2 chunks ahead. Always keep 2 chunks ahead in
    // cache.
    SINT frameCountToCache = 2 * CachingReaderChunk::kFrames;
    current_position.frameCount = frameCountToCache;

    // this called after the precious chunk was consumed
    if (in_reverse) {
        current_position.frame =
                static_cast<SINT>(ceil(m_currentPosition / channelCount)) -
                frameCountToCache;
    } else {
        current_position.frame =
                static_cast<SINT>(floor(m_currentPosition / channelCount));
    }

    // If we are trying to cache before the start of the track,
    // Then we don't need to cache because it's all zeros!
    if (current_position.frame < 0 &&
            current_position.frame + current_position.frameCount < 0)
    {
    	return;
    }

    // top priority, we need to read this data immediately
    current_position.type = Hint::Type::CurrentPosition;
    pHintList->append(current_position);
}

// Not thread-save, call from engine thread only
void ReadAheadManager::addReadLogEntry(double virtualPlaypositionStart,
                                       double virtualPlaypositionEndNonInclusive) {
    ReadLogEntry newEntry(virtualPlaypositionStart,
                          virtualPlaypositionEndNonInclusive);
    if (m_readAheadLog.size() > 0) {
        ReadLogEntry& last = m_readAheadLog.back();
        if (last.merge(newEntry)) {
            return;
        }
    }
    m_readAheadLog.push_back(newEntry);
}

// Not thread-save, call from engine thread only
double ReadAheadManager::getFilePlaypositionFromLog(
        double currentFilePlayposition,
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
    while (m_readAheadLog.size() > 0 && numConsumedSamples > 0) {
        ReadLogEntry& entry = m_readAheadLog.front();
        // Advance our idea of the current virtual playposition to this
        // ReadLogEntry's start position.
        filePlayposition = entry.advancePlayposition(&numConsumedSamples);

        if (entry.length() == 0) {
            // This entry is empty now.
            m_readAheadLog.pop_front();
        }
    }

    return filePlayposition;
}

mixxx::audio::FramePos ReadAheadManager::getFilePlaypositionFromLog(
        mixxx::audio::FramePos currentPosition,
        mixxx::audio::FrameDiff_t numConsumedFrames,
        mixxx::audio::ChannelCount channelCount) {
    const double positionSamples =
            getFilePlaypositionFromLog(currentPosition.toSamplePos(channelCount),
                    numConsumedFrames * channelCount);
    return mixxx::audio::FramePos::fromSamplePos(positionSamples, channelCount);
}
