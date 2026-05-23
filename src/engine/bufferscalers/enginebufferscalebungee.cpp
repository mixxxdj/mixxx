#include "engine/bufferscalers/enginebufferscalebungee.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalebungee.cpp"
#include "util/assert.h"
#include "util/sample.h"
#include "util/timer.h"

EngineBufferScaleBungee::EngineBufferScaleBungee(ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_pStretcher(nullptr),
          m_bBackwards(false),
          m_channelStride(0),
          m_bufferedInputBeginFrame(0),
          m_bufferedInputEndFrame(0),
          m_bResetNeeded(true),
          m_remainingOutputFrames(0),
          m_outputChunkConsumed(0),
          m_inputBufferFrames(0) {
    m_request.position = std::numeric_limits<double>::quiet_NaN();
    m_request.speed = 1.0;
    m_request.pitch = 1.0;
    m_request.reset = false;
    m_request.resampleMode = resampleMode_autoOut;

    m_outputChunk.data = nullptr;
    m_outputChunk.frameCount = 0;
    m_outputChunk.channelStride = 0;
    m_outputChunk.request[0] = nullptr;
    m_outputChunk.request[1] = nullptr;

    m_currentInputChunk.begin = 0;
    m_currentInputChunk.end = 0;

    onSignalChanged();
}

void EngineBufferScaleBungee::onSignalChanged() {
    const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());
    if (m_channelBufferPtrs.size() != static_cast<size_t>(channelCount)) {
        m_channelBufferPtrs.resize(channelCount);
    }

    m_pStretcher.reset();
    m_channelStride = 0;

    if (!getOutputSignal().isValid() || channelCount <= 0) {
        m_inputBufferFrames = 2 * kMaxGrainFrames;
        if (channelCount > 0) {
            m_contiguousChannelBuffer =
                    mixxx::SampleBuffer(m_inputBufferFrames * channelCount);
            for (int ch = 0; ch < channelCount; ++ch) {
                m_channelBufferPtrs[ch] =
                        m_contiguousChannelBuffer.data() + (ch * m_inputBufferFrames);
            }
        }
        m_interleavedReadBuffer =
                mixxx::SampleBuffer(std::max(channelCount, 1) * kMaxGrainFrames);
        clear();
        return;
    }

    const int sampleRate = static_cast<int>(getOutputSignal().getSampleRate());
    Bungee::SampleRates sampleRates;
    sampleRates.input = sampleRate;
    sampleRates.output = sampleRate;

    m_pStretcher = std::make_unique<Bungee::Stretcher<Bungee::Basic>>(
            sampleRates, channelCount, 0);

    // Keep enough room for the largest grain plus an additional read-ahead chunk.
    m_inputBufferFrames = std::max<SINT>(
                                  m_pStretcher->maxInputFrameCount(),
                                  kMaxGrainFrames) +
            kMaxGrainFrames;
    m_channelStride = m_inputBufferFrames;
    m_contiguousChannelBuffer = mixxx::SampleBuffer(m_inputBufferFrames * channelCount);
    // Zero-initialise so Bungee’s Eigen map never reads uninitialised floats
    // during the muted head/tail of the very first grain (position = 0,
    // inputChunk.begin = −halfFrames: the active data only covers the top half
    // of the grain).
    SampleUtil::clear(m_contiguousChannelBuffer.data(),
            m_inputBufferFrames * channelCount);
    for (int ch = 0; ch < channelCount; ++ch) {
        m_channelBufferPtrs[ch] =
                m_contiguousChannelBuffer.data() + (ch * m_inputBufferFrames);
    }

    m_interleavedReadBuffer = mixxx::SampleBuffer(kMaxGrainFrames * channelCount);
    clear();
}

void EngineBufferScaleBungee::setScaleParameters(double base_rate,
        double* pTempoRatio,
        double* pPitchRatio) {
    const bool wasBackwards = m_bBackwards;
    m_bBackwards = *pTempoRatio < 0;

    double speedAbs = fabs(*pTempoRatio);
    if (speedAbs > MAX_SEEK_SPEED) {
        speedAbs = MAX_SEEK_SPEED;
    } else if (speedAbs < MIN_SEEK_SPEED) {
        speedAbs = 0.0;
    }

    *pTempoRatio = m_bBackwards ? -speedAbs : speedAbs;

    m_dBaseRate = base_rate;
    m_dTempoRatio = speedAbs;
    m_dPitchRatio = *pPitchRatio;
    m_effectiveRate = m_dBaseRate * m_dTempoRatio;

    const double pitchScale = fabs(base_rate * *pPitchRatio);
    if (pitchScale > 0.0) {
        m_request.pitch = pitchScale;
    }

    m_request.speed = m_bBackwards ? -m_dTempoRatio : m_dTempoRatio;

    if (wasBackwards != m_bBackwards) {
        m_bResetNeeded = true;
    }
}

void EngineBufferScaleBungee::deinterleaveInput(
        const CSAMPLE* pBuffer,
        SINT destOffsetFrames,
        SINT frames) {
    const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());
    if (channelCount <= 0 || frames <= 0) {
        return;
    }

    DEBUG_ASSERT(destOffsetFrames >= 0);
    DEBUG_ASSERT(destOffsetFrames + frames <= m_channelStride);

    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        SampleUtil::deinterleaveBuffer(
                m_channelBufferPtrs[0] + destOffsetFrames,
                m_channelBufferPtrs[1] + destOffsetFrames,
                pBuffer,
                frames);
        break;
    default: {
        for (SINT frame = 0; frame < frames; ++frame) {
            for (int ch = 0; ch < channelCount; ++ch) {
                m_channelBufferPtrs[ch][destOffsetFrames + frame] =
                        pBuffer[frame * channelCount + ch];
            }
        }
    } break;
    }
}

void EngineBufferScaleBungee::discardBufferedInputBefore(SINT framePosition) {
    if (framePosition <= m_bufferedInputBeginFrame) {
        return;
    }

    const SINT bufferedFrames = m_bufferedInputEndFrame - m_bufferedInputBeginFrame;
    if (bufferedFrames <= 0) {
        m_bufferedInputBeginFrame = framePosition;
        m_bufferedInputEndFrame = framePosition;
        return;
    }

    const SINT discardFrames = std::min(framePosition - m_bufferedInputBeginFrame,
            bufferedFrames);
    const SINT remainingFrames = bufferedFrames - discardFrames;
    for (float* pChannel : m_channelBufferPtrs) {
        std::memmove(pChannel,
                pChannel + discardFrames,
                remainingFrames * sizeof(float));
    }

    m_bufferedInputBeginFrame += discardFrames;
    if (remainingFrames <= 0) {
        // Advance all the way to framePosition, not just to the old
        // m_bufferedInputEndFrame.  Without this, a large gap between the
        // buffer tail and framePosition (e.g. at very high playback speeds)
        // leaves m_bufferedInputBeginFrame too low, making dataOffset
        // incorrectly large in processGrain and causing Bungee's Eigen map
        // to reach past the end of m_contiguousChannelBuffer — heap
        // corruption that is detected later by malloc in an unrelated thread.
        m_bufferedInputBeginFrame = framePosition;
        m_bufferedInputEndFrame = framePosition;
    }
}

SINT EngineBufferScaleBungee::appendInputFrames(
        double signedEffectiveRate,
        SINT framesToRead) {
    if (framesToRead <= 0 || !m_pReadAheadManager) {
        return 0;
    }

    const SINT bufferedFrames = m_bufferedInputEndFrame - m_bufferedInputBeginFrame;
    const SINT availableCapacity = m_inputBufferFrames - bufferedFrames;
    const SINT framesRequested = std::min(framesToRead,
            std::min(availableCapacity, kMaxGrainFrames));
    if (framesRequested <= 0) {
        return 0;
    }

    const SINT samplesRequested = getOutputSignal().frames2samples(framesRequested);
    const SINT availableSamples = m_pReadAheadManager->getNextSamples(
            signedEffectiveRate,
            m_interleavedReadBuffer.data(),
            samplesRequested,
            getOutputSignal().getChannelCount());
    const SINT availableFrames = getOutputSignal().samples2frames(availableSamples);
    if (availableFrames <= 0) {
        return 0;
    }

    deinterleaveInput(m_interleavedReadBuffer.data(), bufferedFrames, availableFrames);
    m_bufferedInputEndFrame += availableFrames;
    return availableFrames;
}

SINT EngineBufferScaleBungee::ensureInputForCurrentChunk(double signedEffectiveRate) {
    if (m_currentInputChunk.end <= m_currentInputChunk.begin) {
        return 0;
    }

    if (m_bufferedInputBeginFrame < m_currentInputChunk.begin) {
        discardBufferedInputBefore(m_currentInputChunk.begin);
    }

    while (m_bufferedInputEndFrame < m_currentInputChunk.end) {
        const SINT missingFrames = m_currentInputChunk.end - m_bufferedInputEndFrame;
        if (appendInputFrames(signedEffectiveRate, missingFrames) <= 0) {
            break;
        }
    }

    const SINT availableBegin = std::max(m_bufferedInputBeginFrame,
            static_cast<SINT>(m_currentInputChunk.begin));
    const SINT availableEnd = std::max(availableBegin,
            std::min(m_bufferedInputEndFrame,
                    static_cast<SINT>(m_currentInputChunk.end)));
    return availableEnd - availableBegin;
}

void EngineBufferScaleBungee::copyOutputFrames(
        CSAMPLE* pDest, SINT offsetInChunk, SINT nFrames) const {
    DEBUG_ASSERT(m_outputChunk.data != nullptr);
    DEBUG_ASSERT(nFrames > 0);
    DEBUG_ASSERT(offsetInChunk + nFrames <=
            static_cast<SINT>(m_outputChunk.frameCount));

    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        // Optimised stereo path: Bungee output is planar (ch0, ch1 separated by
        // channelStride); interleaveBuffer packs them into the output buffer.
        SampleUtil::interleaveBuffer(
                pDest,
                m_outputChunk.data + offsetInChunk,
                m_outputChunk.data + offsetInChunk + m_outputChunk.channelStride,
                nFrames);
        break;
    default: {
        const int channelCount =
                static_cast<int>(getOutputSignal().getChannelCount());
        for (SINT frame = 0; frame < nFrames; ++frame) {
            for (int ch = 0; ch < channelCount; ++ch) {
                pDest[frame * channelCount + ch] =
                        m_outputChunk.data[offsetInChunk + frame +
                                ch * m_outputChunk.channelStride];
            }
        }
    } break;
    }
}

SINT EngineBufferScaleBungee::processGrain(CSAMPLE* pOutputBuffer, SINT maxFrames) {
    if (!m_pStretcher || !getOutputSignal().isValid()) {
        return 0;
    }

    if (m_remainingOutputFrames > 0 && m_outputChunk.data != nullptr) {
        const SINT framesToCopy = std::min(m_remainingOutputFrames, maxFrames);
        const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());

        for (SINT frame = 0; frame < framesToCopy; ++frame) {
            for (int ch = 0; ch < channelCount; ++ch) {
                pOutputBuffer[frame * channelCount + ch] =
                        m_outputChunk.data[m_outputChunkConsumed + frame +
                                ch * m_outputChunk.channelStride];
            }
        }

        m_outputChunkConsumed += framesToCopy;
        m_remainingOutputFrames -= framesToCopy;
        if (m_remainingOutputFrames <= 0) {
            m_outputChunkConsumed = 0;
        }
        return framesToCopy;
    }

    const double signedSpeed = m_bBackwards ? -m_dTempoRatio : m_dTempoRatio;
    const double signedEffectiveRate = (m_bBackwards ? -1.0 : 1.0) * m_effectiveRate;

    if (m_bResetNeeded) {
        m_request.position = 0.0;
        m_request.reset = true;
        m_bResetNeeded = false;
        m_currentInputChunk.begin = 0;
        m_currentInputChunk.end = 0;
        m_bufferedInputBeginFrame = 0;
        m_bufferedInputEndFrame = 0;
    } else {
        m_request.reset = false;
        if (std::isnan(m_request.position)) {
            m_request.position = 0.0;
        }
    }

    m_request.speed = signedSpeed;

    m_currentInputChunk = m_pStretcher->specifyGrain(m_request);
    const SINT framesNeeded = m_currentInputChunk.end - m_currentInputChunk.begin;
    if (framesNeeded <= 0) {
        return 0;
    }

    ensureInputForCurrentChunk(signedEffectiveRate);

    const SINT availableBegin = std::max(m_bufferedInputBeginFrame,
            static_cast<SINT>(m_currentInputChunk.begin));
    const SINT availableEnd = std::max(availableBegin,
            std::min(m_bufferedInputEndFrame,
                    static_cast<SINT>(m_currentInputChunk.end)));
    const int muteHead = availableBegin - m_currentInputChunk.begin;
    const int muteTail = m_currentInputChunk.end - availableEnd;
    const SINT dataOffset = std::max<SINT>(0, availableBegin - m_bufferedInputBeginFrame);

    // Safety guard: dataOffset + grainSize must fit inside the per-channel
    // buffer (capacity = m_channelStride).  If the invariant is broken (e.g.
    // because discardBufferedInputBefore jumped the begin pointer too far),
    // force a reset rather than letting Bungee's Eigen map read past the end
    // of m_contiguousChannelBuffer.
    const SINT grainSize = static_cast<SINT>(
            m_currentInputChunk.end - m_currentInputChunk.begin);
    if (dataOffset + grainSize > m_channelStride) {
        m_bResetNeeded = true;
        return 0;
    }
    DEBUG_ASSERT(m_channelBufferPtrs.empty() || dataOffset + grainSize <= m_channelStride);
    m_pStretcher->analyseGrain(
            m_channelBufferPtrs.empty() ? nullptr : m_channelBufferPtrs[0] + dataOffset,
            m_channelStride,
            muteHead,
            muteTail);
    m_pStretcher->synthesiseGrain(m_outputChunk);
    m_pStretcher->next(m_request);
    m_request.speed = signedSpeed;

    if (m_outputChunk.frameCount <= 0 || m_outputChunk.data == nullptr) {
        return 0;
    }

    m_remainingOutputFrames = m_outputChunk.frameCount;
    m_outputChunkConsumed = 0;

    const SINT framesToCopy = std::min(static_cast<SINT>(m_outputChunk.frameCount), maxFrames);
    copyOutputFrames(pOutputBuffer, 0, framesToCopy);
    m_outputChunkConsumed = framesToCopy;
    m_remainingOutputFrames = m_outputChunk.frameCount - framesToCopy;
    if (m_remainingOutputFrames <= 0) {
        m_outputChunkConsumed = 0;
    }

    return framesToCopy;
}

double EngineBufferScaleBungee::scaleBuffer(CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    if (!m_pStretcher || m_dBaseRate == 0.0 || m_dTempoRatio == 0.0 ||
            !getOutputSignal().isValid()) {
        SampleUtil::clear(pOutputBuffer, iOutputBufferSize);
        return 0.0;
    }

    ScopedTimer t(QStringLiteral("EngineBufferScaleBungee::scaleBuffer"));

    double readFramesProcessed = 0.0;
    SINT remainingFrames = getOutputSignal().samples2frames(iOutputBufferSize);
    CSAMPLE* pOutput = pOutputBuffer;
    bool lastProcessFailed = false;

    while (remainingFrames > 0) {
        const SINT framesProduced = processGrain(pOutput, remainingFrames);
        if (framesProduced > 0) {
            remainingFrames -= framesProduced;
            pOutput += getOutputSignal().frames2samples(framesProduced);
            readFramesProcessed += m_effectiveRate * framesProduced;
            lastProcessFailed = false;
            continue;
        }

        if (lastProcessFailed) {
            if (!m_pStretcher->isFlushed()) {
                Bungee::Request flushRequest{};
                flushRequest.position = std::numeric_limits<double>::quiet_NaN();
                flushRequest.speed = m_request.speed;
                flushRequest.pitch = m_request.pitch;
                flushRequest.reset = true;
                flushRequest.resampleMode = resampleMode_autoOut;

                m_pStretcher->specifyGrain(flushRequest);
                m_pStretcher->synthesiseGrain(m_outputChunk);

                if (m_outputChunk.frameCount > 0 && m_outputChunk.data != nullptr) {
                    const SINT framesToCopy = std::min(
                            static_cast<SINT>(m_outputChunk.frameCount),
                            remainingFrames);
                    const int channelCount = static_cast<int>(
                            getOutputSignal().getChannelCount());
                    for (SINT frame = 0; frame < framesToCopy; ++frame) {
                        for (int ch = 0; ch < channelCount; ++ch) {
                            pOutput[frame * channelCount + ch] =
                                    m_outputChunk.data[frame +
                                            ch * m_outputChunk.channelStride];
                        }
                    }

                    remainingFrames -= framesToCopy;
                    pOutput += getOutputSignal().frames2samples(framesToCopy);
                }
            }

            if (remainingFrames > 0) {
                SampleUtil::clear(
                        pOutput, getOutputSignal().frames2samples(remainingFrames));
            }
            break;
        }

        lastProcessFailed = true;
    }

    return readFramesProcessed;
}

void EngineBufferScaleBungee::clear() {
    m_bResetNeeded = true;
    m_remainingOutputFrames = 0;
    m_outputChunkConsumed = 0;
    m_currentInputChunk.begin = 0;
    m_currentInputChunk.end = 0;
    m_bufferedInputBeginFrame = 0;
    m_bufferedInputEndFrame = 0;

    m_request.position = std::numeric_limits<double>::quiet_NaN();
    m_request.speed = 1.0;
    m_request.pitch = 1.0;
    m_request.reset = true;
    m_request.resampleMode = resampleMode_autoOut;

    m_effectiveRate = m_dBaseRate * m_dTempoRatio;

    m_outputChunk.data = nullptr;
    m_outputChunk.frameCount = 0;
    m_outputChunk.channelStride = 0;
    m_outputChunk.request[0] = nullptr;
    m_outputChunk.request[1] = nullptr;
}
