#include "engine/bufferscalers/enginebufferscalesignalsmith.h"

#include "engine/engine.h"
#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesignalsmith.cpp"
#include "util/assert.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/timer.h"

EngineBufferScaleSignalSmith::EngineBufferScaleSignalSmith(ReadAheadManager* pReadAheadManager)
        : m_pReadAheadManager(pReadAheadManager),
          m_buffers(),
          m_bufferPtrs(),
          m_interleavedBuffer(mixxx::kMaxSupportedStems * MAX_BUFFER_LEN),
          m_frameFractionalLeftover(0),
          m_expectedFrameLatency(0),
          m_currentFrameOffset(0),
          m_currentPreset(Preset::Default) {
    onSignalChanged();
}

void EngineBufferScaleSignalSmith::setScaleParameters(
        double base_rate, double* pTempoRatio, double* pPitchRatio) {
    m_dBaseRate = base_rate;
    m_bBackwards = *pTempoRatio < 0;
    m_dTempoRatio = std::fabs(*pTempoRatio);
    m_dPitchRatio = *pPitchRatio;
    m_effectiveRate = m_dBaseRate * m_dTempoRatio;

    m_stretch.setTransposeFactor(static_cast<float>(m_dBaseRate * m_dPitchRatio));
    m_stretch.setFormantFactor(1.0);

    // The following value is calculated from the block and interval samples
    // size, which are set in the above preset. It remains constant during the
    // stretcher process.
    // As documented in
    // https://signalsmith-audio.co.uk/code/stretch/#how-to-use-latency-starting-and-ending,
    // stretch factor should be used when computing total latency
    m_expectedFrameLatency =
            static_cast<SINT>(std::round(m_effectiveRate *
                    static_cast<double>(m_stretch.inputLatency()))) +
            static_cast<SINT>(m_stretch.outputLatency());
}

void EngineBufferScaleSignalSmith::onSignalChanged() {
    if (!getOutputSignal().isValid()) {
        return;
    }

    uint8_t channelCount = getOutputSignal().getChannelCount();
    if (m_buffers.size() != channelCount) {
        m_buffers.resize(channelCount);
    }

    if (m_bufferPtrs.size() != channelCount) {
        m_bufferPtrs.resize(channelCount);
    }

    for (int chIdx = 0; chIdx < channelCount; chIdx++) {
        if (m_buffers[chIdx].size() == MAX_BUFFER_LEN) {
            continue;
        }
        m_buffers[chIdx] = mixxx::SampleBuffer(MAX_BUFFER_LEN);
        m_bufferPtrs[chIdx] = m_buffers[chIdx].data();
    }

    // Configure stretcher with preset settings
    switch (m_currentPreset) {
    case Preset::Cheaper:
        m_stretch.presetCheaper(channelCount, getOutputSignal().getSampleRate());
        break;
    default:
        qWarning() << "Unsupported presset" << m_currentPreset << " so defaulting to default.";
        [[fallthrough]];
    case Preset::Default:
        m_stretch.presetDefault(channelCount, getOutputSignal().getSampleRate());
        break;
    }
    clear();
}

void EngineBufferScaleSignalSmith::clear() {
    m_stretch.reset();
    m_currentFrameOffset = 0;
    m_frameFractionalLeftover = 0;
}

SINT EngineBufferScaleSignalSmith::fetchAndDeinterleave(SINT sampleToRead, SINT frameOffset) {
    auto sampleOffset = getOutputSignal().frames2samples(frameOffset);
    auto frameRead = getOutputSignal().samples2frames(
            m_pReadAheadManager->getNextSamples(
                    // The value doesn't matter here. All that matters is we
                    // are going forward or backward.
                    (m_bBackwards ? -1 : 1) * m_dBaseRate * m_dTempoRatio,
                    m_interleavedBuffer.data(),
                    sampleToRead,
                    getOutputSignal().getChannelCount()));

    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        SampleUtil::deinterleaveBuffer(
                m_buffers[0].data(frameOffset),
                m_buffers[1].data(frameOffset),
                m_interleavedBuffer.data(sampleOffset),
                frameRead);
        break;
    case mixxx::audio::ChannelCount::stem():
        SampleUtil::deinterleaveBuffer(
                m_buffers[0].data(frameOffset),
                m_buffers[1].data(frameOffset),
                m_buffers[2].data(frameOffset),
                m_buffers[3].data(frameOffset),
                m_buffers[4].data(frameOffset),
                m_buffers[5].data(frameOffset),
                m_buffers[6].data(frameOffset),
                m_buffers[7].data(frameOffset),
                m_interleavedBuffer.data(sampleOffset),
                frameRead);
        break;
    default: {
        int chCount = getOutputSignal().getChannelCount();
        // The sampler are ordered as following in pBuffer
        //    1234..X1234...X...
        // And need to be reordered as following
        // m_buffers#1 = 11..
        // m_buffers#2 = 22..
        // m_buffers#3 = 33..
        // m_buffers#4 = 44..fff
        // m_buffers#X = XX..
        //
        // Because of the unanticipated number of buffer and channel, we cannot
        // use any SampleUtil in this case
        for (SINT frameIdx = 0; frameIdx < frameRead; ++frameIdx) {
            for (int channel = 0; channel < chCount; channel++) {
                m_buffers[channel].data(frameOffset)[frameIdx] =
                        m_interleavedBuffer.data(sampleOffset)[frameIdx * chCount + channel];
            }
        }
    } break;
    }
    return frameRead;
}

double EngineBufferScaleSignalSmith::scaleBuffer(CSAMPLE* pOutputBuffer, SINT iOutputBufferSize) {
    ScopedTimer t(QStringLiteral("EngineBufferScaleSignalsmith::scaleBuffer"));

    // Unlike RubberBand, SignalSmith Stretch always output as much audio as it
    // was given. However, it does introduce latency (documented at
    // https://signalsmith-audio.co.uk/code/stretch/#how-to-use-latency) which
    // initially lead to a silence. To compensate that, we need to use the
    // `.outputSeek` method, which allows to pre-roll samples a realign the actual
    // output to real time.
    // However, this method will reset the buffer so it can only be used right after a reset
    if (m_currentFrameOffset == 0 &&
            m_currentFrameOffset < m_expectedFrameLatency
            // If the track has a zero rate, we skip correction as this is
            // usually a sign that the track is not playing. This will likely
            // create undesired silence (as opposite to a "zero BPM" play affect
            // if a track start playing with a zero BPM, but this is an
            // acceptable trade off for now)
            && m_dTempoRatio > 0) {
        const SINT frameRead =
                fetchAndDeinterleave(getOutputSignal().frames2samples(
                        std::min(m_expectedFrameLatency - m_currentFrameOffset,
                                SINT(MAX_BUFFER_LEN))));
        m_stretch.outputSeek(m_bufferPtrs.data(), frameRead);
        m_currentFrameOffset += frameRead;
    }

    const SINT outputFrames = getOutputSignal().samples2frames(iOutputBufferSize);
    auto dFrameRequired =
            (m_dBaseRate * m_dTempoRatio * static_cast<double>(outputFrames)) +
            m_frameFractionalLeftover;

    if (m_currentFrameOffset != m_expectedFrameLatency && dFrameRequired > 0) {
        // This happens when the rate changes because the rate scales the input
        // latency. We need more or less latency frames to keep the output steady.
        // The rate changed is immediately applied to the audio without any glitch.
        // Pitch changes do not affect latency.
        double frameOffset = std::max(-dFrameRequired,
                static_cast<double>(m_expectedFrameLatency - m_currentFrameOffset));
        dFrameRequired += frameOffset;
        m_currentFrameOffset += static_cast<SINT>(frameOffset);
    }

    const SINT frameRequired = static_cast<SINT>(dFrameRequired);
    VERIFY_OR_DEBUG_ASSERT(frameRequired <= MAX_BUFFER_LEN) {
        return 0.0;
    }

    m_frameFractionalLeftover = dFrameRequired - static_cast<double>(frameRequired);
    DEBUG_ASSERT(0 <= m_frameFractionalLeftover && m_frameFractionalLeftover < 1);

    bool last_read_failed = false;
    SINT frameRead = 0;
    while (frameRead < frameRequired) {
        auto currentFrameRead = fetchAndDeinterleave(getOutputSignal().frames2samples(
                                                             frameRequired - frameRead),
                frameRead);
        frameRead += currentFrameRead;

        if (last_read_failed && currentFrameRead <= 0) {
            // flush and break out after
            // the next retrieval. If we are at EOF this serves to get
            // the last samples out of the scaler.
            for (int ch = 0; ch < getOutputSignal().getChannelCount(); ch++) {
                SampleUtil::clear(m_buffers[ch].data(frameRead), frameRequired - frameRead);
            }
            frameRead = frameRequired;
            break;
        } else if (frameRead <= 0) {
            last_read_failed = true;
        }
    }

    DEBUG_ASSERT(frameRead == frameRequired);

    {
        ScopedTimer t(QStringLiteral("Signalsmith::process"));
        float* outputBufferPtr[8] = {
                m_interleavedBuffer.data(),
                m_interleavedBuffer.data(iOutputBufferSize),
                m_interleavedBuffer.data(2 * iOutputBufferSize),
                m_interleavedBuffer.data(3 * iOutputBufferSize),
                m_interleavedBuffer.data(4 * iOutputBufferSize),
                m_interleavedBuffer.data(5 * iOutputBufferSize),
                m_interleavedBuffer.data(6 * iOutputBufferSize),
                m_interleavedBuffer.data(7 * iOutputBufferSize),
        };
        m_stretch.process(m_bufferPtrs.data(), frameRead, outputBufferPtr, outputFrames);
    }

    auto outputFrameSize = getOutputSignal().samples2frames(iOutputBufferSize);
    switch (getOutputSignal().getChannelCount()) {
    case mixxx::audio::ChannelCount::stereo():
        SampleUtil::interleaveBuffer(pOutputBuffer,
                m_interleavedBuffer.data(),
                m_interleavedBuffer.data(iOutputBufferSize),
                outputFrameSize);
        break;
    case mixxx::audio::ChannelCount::stem():
        SampleUtil::interleaveBuffer(pOutputBuffer,
                m_interleavedBuffer.data(),
                m_interleavedBuffer.data(iOutputBufferSize),
                m_interleavedBuffer.data(2 * iOutputBufferSize),
                m_interleavedBuffer.data(3 * iOutputBufferSize),
                m_interleavedBuffer.data(4 * iOutputBufferSize),
                m_interleavedBuffer.data(5 * iOutputBufferSize),
                m_interleavedBuffer.data(6 * iOutputBufferSize),
                m_interleavedBuffer.data(7 * iOutputBufferSize),
                outputFrameSize);
        break;
    default: {
        int chCount = getOutputSignal().getChannelCount();
        // The buffers samples are ordered as following
        //  m_buffers#1 = 11..
        //  m_buffers#2 = 22..
        //  m_buffers#3 = 33..
        //  m_buffers#4 = 44..
        //  m_buffers#X = XX..
        // And need to be reordered as following in pBuffer
        //  1234..X1234...X...
        //
        // Because of the unanticipated number of buffer and channel, we cannot
        // use any SampleUtil in this case
        for (SINT frameIdx = 0;
                frameIdx < getOutputSignal().samples2frames(iOutputBufferSize);
                ++frameIdx) {
            for (int channel = 0; channel < chCount; channel++) {
                pOutputBuffer[frameIdx * chCount + channel] =
                        m_buffers[channel].data()[frameIdx];
            }
        }
    } break;
    }

    // readFramesProcessed is interpreted as the total number of frames
    // consumed to produce the scaled buffer. Due to this, we do not take into
    // account directionality or starting point.
    return m_effectiveRate * outputFrames;
}
