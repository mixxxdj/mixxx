#include "engine/bufferscalers/enginebufferscalebungee.h"

#include <QtDebug>
#include <cmath>
#include <limits>

#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalebungee.cpp"
#include "util/sample.h"
#include "util/timer.h"

EngineBufferScaleBungee::EngineBufferScaleBungee(ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_pStretcher(nullptr),
          m_bBackwards(false),
          m_channelStride(0),
          m_grainPosition(0.0),
          m_bResetNeeded(true),
          m_remainingOutputFrames(0),
          m_outputChunkConsumed(0),
          m_inputBufferFrames(0) {
    // Initialize the request with NaN position (invalid)
    m_request.position = std::numeric_limits<double>::quiet_NaN();
    m_request.speed = 1.0;
    m_request.pitch = 1.0;
    m_request.reset = false;
    m_request.resampleMode = resampleMode_autoOut;

    // Initialize output chunk
    m_outputChunk.data = nullptr;
    m_outputChunk.frameCount = 0;
    m_outputChunk.channelStride = 0;
    m_outputChunk.request[0] = nullptr;
    m_outputChunk.request[1] = nullptr;

    // Initialize input chunk
    m_currentInputChunk.begin = 0;
    m_currentInputChunk.end = 0;

    onSignalChanged();
}

void EngineBufferScaleBungee::onSignalChanged() {
    const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());

    // Only create stretcher if we have a valid signal
    if (!getOutputSignal().isValid()) {
        // Still need to initialize channel buffers with default size
        if (m_channelBuffers.size() != static_cast<size_t>(channelCount)) {
            m_channelBuffers.resize(channelCount);
            m_channelBufferPtrs.resize(channelCount);
        }
        // Use a default size until we have valid signal
        m_inputBufferFrames = kMaxGrainFrames;
        for (int ch = 0; ch < channelCount; ++ch) {
            if (static_cast<size_t>(m_channelBuffers[ch].size()) < static_cast<size_t>(m_inputBufferFrames)) {
                m_channelBuffers[ch] = mixxx::SampleBuffer(m_inputBufferFrames);
            }
            m_channelBufferPtrs[ch] = m_channelBuffers[ch].data();
        }
        return;
    }

    const int sampleRate = static_cast<int>(getOutputSignal().getSampleRate());

    // Create Bungee stretcher with input and output sample rates
    Bungee::SampleRates sampleRates;
    sampleRates.input = sampleRate;
    sampleRates.output = sampleRate;

    m_pStretcher = std::make_unique<Bungee::Stretcher<Bungee::Basic>>(
            sampleRates, channelCount, 0);

    // Get the actual buffer size required by Bungee
    m_inputBufferFrames = m_pStretcher->maxInputFrameCount();

    // Initialize channel buffers - Option A: Single contiguous buffer for all channels
    // This ensures proper channel stride for Bungee's planar format
    if (m_channelBuffers.size() != static_cast<size_t>(channelCount)) {
        m_channelBuffers.resize(channelCount);
        m_channelBufferPtrs.resize(channelCount);
    }

    // Allocate single contiguous buffer for all channels
    const SINT totalFrames = m_inputBufferFrames * channelCount;
    if (static_cast<size_t>(m_contiguousChannelBuffer.size()) < static_cast<size_t>(totalFrames)) {
        m_contiguousChannelBuffer = mixxx::SampleBuffer(totalFrames);
    }

    // Set up channel pointers within the contiguous buffer
    for (int ch = 0; ch < channelCount; ++ch) {
        // Each channel starts at offset ch * m_inputBufferFrames
        m_channelBufferPtrs[ch] = m_contiguousChannelBuffer.data() + (ch * m_inputBufferFrames);
    }

    // Channel stride is the number of frames between consecutive channels
    m_channelStride = m_inputBufferFrames;

    // Initialize interleaved read buffer
    const SINT interleavedSize = m_inputBufferFrames * channelCount;
    if (m_interleavedReadBuffer.size() < interleavedSize) {
        m_interleavedReadBuffer = mixxx::SampleBuffer(interleavedSize);
    }

    // Reset state
    m_request.position = std::numeric_limits<double>::quiet_NaN();
    m_request.reset = true;
    m_bResetNeeded = true;
    m_remainingOutputFrames = 0;
    m_outputChunkConsumed = 0;
    m_grainPosition = 0.0;

    // Initialize output chunk
    m_outputChunk.data = nullptr;
    m_outputChunk.frameCount = 0;
}

void EngineBufferScaleBungee::setScaleParameters(double base_rate,
        double* pTempoRatio,
        double* pPitchRatio) {
    // Negative speed means we are going backwards
    m_bBackwards = *pTempoRatio < 0;

    // Clamp speed to valid range
    double speed_abs = fabs(*pTempoRatio);
    if (speed_abs > MAX_SEEK_SPEED) {
        speed_abs = MAX_SEEK_SPEED;
    } else if (speed_abs < MIN_SEEK_SPEED) {
        speed_abs = 0.0;
    }

    // Let the caller know if we clamped their value
    *pTempoRatio = m_bBackwards ? -speed_abs : speed_abs;

    // Update stored parameters
    m_dBaseRate = base_rate;
    m_dTempoRatio = speed_abs;
    m_dPitchRatio = *pPitchRatio;

    // Calculate effective pitch scale (base_rate * pitch_ratio)
    // Bungee expects pitch as frequency multiplier (1.0 = no change)
    double pitchScale = fabs(base_rate * *pPitchRatio);
    if (pitchScale > 0.0) {
        m_request.pitch = pitchScale;
    }

    // Speed is used by Bungee when it can't determine speed from position deltas
    m_request.speed = m_dBaseRate * m_dTempoRatio;

    // If the direction changed, we need to reset
    if (m_bBackwards && m_request.speed > 0) {
        m_bResetNeeded = true;
    } else if (!m_bBackwards && m_request.speed < 0) {
        m_bResetNeeded = true;
    }
}

void EngineBufferScaleBungee::deinterleaveInput(const CSAMPLE* pBuffer, SINT frames) {
    const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());

    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        SampleUtil::deinterleaveBuffer(
                m_channelBufferPtrs[0],
                m_channelBufferPtrs[1],
                pBuffer,
                frames);
        break;
    default: {
        // Generic deinterleave for any channel count
        for (SINT frame = 0; frame < frames; ++frame) {
            for (int ch = 0; ch < channelCount; ++ch) {
                m_channelBufferPtrs[ch][frame] = pBuffer[frame * channelCount + ch];
            }
        }
    } break;
    }
}

SINT EngineBufferScaleBungee::processGrain(CSAMPLE* pOutputBuffer, SINT maxFrames) {
    if (!m_pStretcher || !getOutputSignal().isValid()) {
        return 0;
    }

    // If we have remaining output from a previous grain, copy it first
    if (m_remainingOutputFrames > 0 && m_outputChunk.data != nullptr) {
        const SINT framesToCopy = std::min(m_remainingOutputFrames, maxFrames);
        const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());

        // Copy from output chunk to interleaved buffer
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

    // Check if we need to reset
    if (m_bResetNeeded) {
        m_request.reset = true;
        m_bResetNeeded = false;
        // Reset grain position when reset is requested to prevent drift
        m_grainPosition = 0.0;
    } else {
        m_request.reset = false;
    }

    // Only process if we have valid parameters
    double speed = m_dBaseRate * m_dTempoRatio;
    if (m_bBackwards) {
        speed = -speed;
    }

    // Calculate the next grain position based on actual consumed input
    // This ensures position tracking stays synchronized with the audio
    if (!std::isnan(m_request.position)) {
        // Advance position based on actual frames consumed in previous iteration
        // Use Bungee's expected input chunk size to maintain synchronization
        const double grainAdvance = static_cast<double>(kMaxGrainFrames);
        m_grainPosition += (m_bBackwards ? -grainAdvance : grainAdvance);
    } else {
        // First grain - start at position 0
        m_grainPosition = 0.0;
    }

    m_request.position = m_grainPosition;
    m_request.speed = speed;

    // Get input requirements for this grain
    m_currentInputChunk = m_pStretcher->specifyGrain(m_request);

    // Calculate how many frames we need to read
    const int framesNeeded = m_currentInputChunk.end - m_currentInputChunk.begin;

    if (framesNeeded <= 0) {
        return 0;
    }

    // Read input from ReadAheadManager
    const SINT samplesNeeded = getOutputSignal().frames2samples(framesNeeded);
    const SINT availableSamples = m_pReadAheadManager->getNextSamples(
            speed,
            m_interleavedReadBuffer.data(),
            samplesNeeded,
            getOutputSignal().getChannelCount());

    const SINT availableFrames = getOutputSignal().samples2frames(availableSamples);

    if (availableFrames <= 0) {
        // No input available - don't advance position to retry at same position
        // This prevents waveform jiggle when data is temporarily unavailable
        return 0;
    }

    // Deinterleave input for Bungee
    deinterleaveInput(m_interleavedReadBuffer.data(), availableFrames);

    // Analyze the grain - pass the base pointer and correct stride
    // With contiguous buffer, channel[0] is at base, channel[1] is at base + m_channelStride, etc.
    const int muteHead = 0;
    const int muteTail = framesNeeded - availableFrames;
    m_pStretcher->analyseGrain(
            m_channelBufferPtrs[0],
            m_channelStride,
            muteHead,
            std::max(0, muteTail));

    // Synthesize output
    m_pStretcher->synthesiseGrain(m_outputChunk);

    if (m_outputChunk.frameCount <= 0) {
        return 0;
    }

    // Store output info for copying
    m_remainingOutputFrames = m_outputChunk.frameCount;
    m_outputChunkConsumed = 0;

    // Copy output to buffer (limited by maxFrames)
    const SINT framesToCopy = std::min(static_cast<SINT>(m_outputChunk.frameCount), maxFrames);
    const int channelCount = static_cast<int>(getOutputSignal().getChannelCount());

    for (SINT frame = 0; frame < framesToCopy; ++frame) {
        for (int ch = 0; ch < channelCount; ++ch) {
            pOutputBuffer[frame * channelCount + ch] =
                    m_outputChunk.data[frame + ch * m_outputChunk.channelStride];
        }
    }

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

    bool lastReadFailed = false;

    while (remainingFrames > 0) {
        // Process grains until we fill the output buffer
        SINT framesProduced = processGrain(pOutput, remainingFrames);

        if (framesProduced > 0) {
            remainingFrames -= framesProduced;
            pOutput += getOutputSignal().frames2samples(framesProduced);
            readFramesProcessed += m_dBaseRate * m_dTempoRatio * framesProduced;
            lastReadFailed = false;
        } else {
            // No frames produced - we may be at EOF or need more input
            if (lastReadFailed) {
                // Flush any remaining output from Bungee
                if (!m_pStretcher->isFlushed()) {
                    // Create a flush request
                    Bungee::Request flushRequest;
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
                                pOutput[frame * channelCount +
                                        ch] = m_outputChunk.data[frame +
                                        ch * m_outputChunk.channelStride];
                            }
                        }

                        remainingFrames -= framesToCopy;
                        pOutput += getOutputSignal().frames2samples(framesToCopy);
                    }
                }

                // Clear any remaining buffer
                if (remainingFrames > 0) {
                    SampleUtil::clear(pOutput,
                            getOutputSignal().frames2samples(remainingFrames));
                }
                break;
            }
            lastReadFailed = true;

            // Try to get more input
            const SINT samplesToRead = getOutputSignal().frames2samples(kMaxGrainFrames);
            const SINT availableSamples = m_pReadAheadManager->getNextSamples(
                    (m_bBackwards ? -1.0 : 1.0) * m_dBaseRate * m_dTempoRatio,
                    m_interleavedReadBuffer.data(),
                    samplesToRead,
                    getOutputSignal().getChannelCount());

            if (availableSamples <= 0) {
                // No more input available
                continue;
            }
        }
    }

    return readFramesProcessed;
}

void EngineBufferScaleBungee::clear() {
    m_bResetNeeded = true;
    m_remainingOutputFrames = 0;
    m_outputChunkConsumed = 0;
    m_request.position = std::numeric_limits<double>::quiet_NaN();
    m_request.speed = 1.0;
    m_request.pitch = 1.0;
    m_request.reset = true;
    m_request.resampleMode = resampleMode_autoOut;
    m_grainPosition = 0.0;

    // Reset output chunk state to prevent use of stale data
    m_outputChunk.data = nullptr;
    m_outputChunk.frameCount = 0;

    // Note: We don't call specifyGrain on the stretcher here because
    // it can cause crashes when switching engines while playing.
    // The reset flag in m_request will be processed on the next processGrain call.
}
