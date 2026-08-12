#include "engine/bufferscalers/enginebufferscalebungee.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalebungee.cpp"
#include "util/assert.h"
#include "util/fpclassify.h"
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
          m_lastReadFramesProcessed(0.0),
          m_outputLatencyFrames(0),
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
    m_outputLatencyFrames = 0;

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
    const int log2SynthesisHop = std::max(
            0,
            static_cast<int>(std::bit_width(static_cast<unsigned>(sampleRate))) -
                    1 -
                    6);
    m_outputLatencyFrames = 2 * (SINT{1} << log2SynthesisHop);

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

double EngineBufferScaleBungee::getVisualPlayPositionOffset() const {
    const double signedRate = m_bBackwards ? -m_effectiveRate : m_effectiveRate;
    return -signedRate * static_cast<double>(m_outputLatencyFrames);
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

    m_dBaseRate = util_isfinite(base_rate) ? std::fabs(base_rate) : 0.0;
    m_dTempoRatio = speedAbs;
    m_dPitchRatio = *pPitchRatio;
    const double requestedEffectiveRate = m_dBaseRate * m_dTempoRatio;
    if (m_remainingOutputFrames <= 0) {
        m_effectiveRate = requestedEffectiveRate;
    }

    const double pitchScale = fabs(m_dBaseRate * *pPitchRatio);
    if (util_isfinite(pitchScale) && pitchScale > 0.0) {
        m_request.pitch = pitchScale;
    } else {
        m_request.pitch = 1.0;
    }

    m_request.speed = m_bBackwards ? -requestedEffectiveRate : requestedEffectiveRate;

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

SINT EngineBufferScaleBungee::consumeReadAheadGap(
        double signedEffectiveRate,
        SINT framesToConsume) {
    if (framesToConsume <= 0) {
        return 0;
    }
    if (!m_pReadAheadManager || !getOutputSignal().isValid()) {
        return framesToConsume;
    }

    SINT consumedFrames = 0;
    int readFailedCount = 0;
    while (consumedFrames < framesToConsume) {
        const SINT framesRequested = std::min<SINT>(
                framesToConsume - consumedFrames,
                kMaxGrainFrames);
        const SINT samplesRequested = getOutputSignal().frames2samples(framesRequested);
        const SINT availableSamples = m_pReadAheadManager->getNextSamples(
                signedEffectiveRate,
                m_interleavedReadBuffer.data(),
                samplesRequested,
                getOutputSignal().getChannelCount());
        const SINT availableFrames = getOutputSignal().samples2frames(availableSamples);
        if (availableFrames <= 0) {
            if (++readFailedCount > 1) {
                break;
            }
            continue;
        }
        readFailedCount = 0;
        consumedFrames += std::min(availableFrames, framesRequested);
    }

    return consumedFrames;
}

void EngineBufferScaleBungee::discardBufferedInputBefore(
        SINT framePosition,
        double signedEffectiveRate) {
    if (framePosition <= m_bufferedInputBeginFrame) {
        return;
    }

    const SINT bufferedFrames = m_bufferedInputEndFrame - m_bufferedInputBeginFrame;
    if (bufferedFrames <= 0) {
        const SINT consumedGapFrames = consumeReadAheadGap(
                signedEffectiveRate,
                framePosition - m_bufferedInputEndFrame);
        m_bufferedInputBeginFrame = m_bufferedInputEndFrame + consumedGapFrames;
        m_bufferedInputEndFrame = m_bufferedInputBeginFrame;
        return;
    }

    const SINT oldBufferedInputEndFrame = m_bufferedInputEndFrame;
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
        const SINT consumedGapFrames = consumeReadAheadGap(
                signedEffectiveRate,
                framePosition - oldBufferedInputEndFrame);
        // Advance beyond the old m_bufferedInputEndFrame only after consuming
        // the skipped source gap from ReadAheadManager. This keeps future
        // appendInputFrames() calls from labeling old sequential samples with
        // this future absolute frame.
        m_bufferedInputBeginFrame = oldBufferedInputEndFrame + consumedGapFrames;
        m_bufferedInputEndFrame = m_bufferedInputBeginFrame;
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
        discardBufferedInputBefore(m_currentInputChunk.begin, signedEffectiveRate);
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

bool EngineBufferScaleBungee::hasValidOutputChunk() const {
    return m_outputChunk.frameCount > 0 &&
            m_outputChunk.data != nullptr;
}

double EngineBufferScaleBungee::copyFlushOutputFrames(
        CSAMPLE*& pOutput,
        SINT& remainingFrames) const {
    if (!hasValidOutputChunk()) {
        return 0.0;
    }

    const SINT framesToCopy = std::min(
            static_cast<SINT>(m_outputChunk.frameCount),
            remainingFrames);
    if (framesToCopy <= 0) {
        return 0.0;
    }
    copyOutputFrames(pOutput, 0, framesToCopy);

    remainingFrames -= framesToCopy;
    pOutput += getOutputSignal().frames2samples(framesToCopy);
    return m_effectiveRate * static_cast<double>(framesToCopy);
}

SINT EngineBufferScaleBungee::processGrain(CSAMPLE* pOutputBuffer, SINT maxFrames) {
    m_lastReadFramesProcessed = 0.0;

    if (!m_pStretcher || !getOutputSignal().isValid()) {
        return 0;
    }

    if (m_remainingOutputFrames > 0 && m_outputChunk.data != nullptr) {
        if (!hasValidOutputChunk()) {
            m_remainingOutputFrames = 0;
            m_outputChunkConsumed = 0;
            return 0;
        }

        const SINT framesToCopy = std::min(m_remainingOutputFrames, maxFrames);
        m_lastReadFramesProcessed =
                m_effectiveRate * static_cast<double>(framesToCopy);
        copyOutputFrames(pOutputBuffer, m_outputChunkConsumed, framesToCopy);

        m_outputChunkConsumed += framesToCopy;
        m_remainingOutputFrames -= framesToCopy;
        if (m_remainingOutputFrames <= 0) {
            m_outputChunkConsumed = 0;
        }
        return framesToCopy;
    }

    m_effectiveRate = m_dBaseRate * m_dTempoRatio;
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
        if (util_isnan(m_request.position)) {
            m_request.position = 0.0;
        }
    }

    m_request.speed = signedEffectiveRate;

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
    m_request.speed = signedEffectiveRate;

    if (!hasValidOutputChunk()) {
        return 0;
    }

    m_remainingOutputFrames = m_outputChunk.frameCount;
    m_outputChunkConsumed = 0;

    const SINT framesToCopy = std::min(static_cast<SINT>(m_outputChunk.frameCount), maxFrames);
    m_lastReadFramesProcessed = m_effectiveRate * static_cast<double>(framesToCopy);
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
            readFramesProcessed += m_lastReadFramesProcessed;
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

                readFramesProcessed +=
                        copyFlushOutputFrames(pOutput, remainingFrames);
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
    m_lastReadFramesProcessed = 0.0;
    m_currentInputChunk.begin = 0;
    m_currentInputChunk.end = 0;
    m_bufferedInputBeginFrame = 0;
    m_bufferedInputEndFrame = 0;

    m_effectiveRate = m_dBaseRate * m_dTempoRatio;

    m_request.position = std::numeric_limits<double>::quiet_NaN();
    m_request.speed = m_bBackwards ? -m_effectiveRate : m_effectiveRate;
    const double pitchScale = fabs(m_dBaseRate * m_dPitchRatio);
    m_request.pitch = util_isfinite(pitchScale) && pitchScale > 0.0
            ? pitchScale
            : 1.0;
    m_request.reset = true;
    m_request.resampleMode = resampleMode_autoOut;

    m_outputChunk.data = nullptr;
    m_outputChunk.frameCount = 0;
    m_outputChunk.channelStride = 0;
    m_outputChunk.request[0] = nullptr;
    m_outputChunk.request[1] = nullptr;
}
