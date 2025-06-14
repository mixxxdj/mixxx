#include "engine/bufferscalers/enginebufferscalesr.h"

#include <QDebug>

#include "engine/enginebuffer.h"
#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesr.cpp"
#include "util/math.h"
#include "util/sample.h"

namespace {
constexpr SINT kBackBufferFrameSize = 512;
}

EngineBufferScaleSR::EngineBufferScaleSR(ReadAheadManager* pReadAheadManager, double eIndex)
        : m_pReadAheadManager(pReadAheadManager),
          m_bBackwards(false),
          m_pResampler(nullptr) {
    setQuality(eIndex);
}

EngineBufferScaleSR::~EngineBufferScaleSR() {
}

void EngineBufferScaleSR::setQuality(double engine_quality) {
    if (m_pResampler) {
        src_delete(m_pResampler);
    }

    auto quality = static_cast<EngineBuffer::ScratchingEngine>(engine_quality);

    uint8_t num_channels = getOutputSignal().getChannelCount().value();

    int error;
    if (quality == EngineBuffer::ScratchingEngine::SampleRateLinear) {
        m_pResampler = src_new(SRC_LINEAR, num_channels, &error);
    } else if (quality == EngineBuffer::ScratchingEngine::SampleRateSincFinest) {
        m_pResampler = src_new(SRC_SINC_BEST_QUALITY, num_channels, &error);
    } else if (quality == EngineBuffer::ScratchingEngine::SampleRateSincFastest) {
        m_pResampler = src_new(SRC_SINC_FASTEST, num_channels, &error);
    }
}

// these parameters describe the "request" that
// needs to be handled by libsamplerate.
// use the default implementation for now
void EngineBufferScaleSR::setScaleParameters(double base_rate,
        double* pTempoRatio,
        double* pPitchRatio) {
    // Negative speed means we are going backwards. pitch does not affect
    // the playback direction.
    m_bBackwards = *pTempoRatio < 0;

    // seek speed limits
    double speed_abs = fabs(*pTempoRatio);
    if (speed_abs > MAX_SEEK_SPEED) {
        speed_abs = MAX_SEEK_SPEED;
    } else if (speed_abs < MIN_SEEK_SPEED) {
        speed_abs = 0;
    }

    // clamp tempo
    *pTempoRatio = m_bBackwards ? -speed_abs : speed_abs;

    EngineBufferScale::setScaleParameters(base_rate, pTempoRatio, pPitchRatio);

    // Reset resampler state when parameters change significantly
    // libsamplerate only handles tempo/rate changes, not pitch
    if (m_pResampler && (base_rate != m_dBaseRate || speed_abs != fabs(m_dTempoRatio))) {
        src_reset(m_pResampler);
    }

    // Calculate effective rate for use in scaleBuffer()
    m_effectiveRate = m_dBaseRate * fabs(m_dTempoRatio);
}

void EngineBufferScaleSR::clear() {
    if (m_pResampler) {
        src_reset(m_pResampler);
    }

    // Clear our input buffer
    if (!m_bufferBack.size() > 0) {
        SampleUtil::clear(m_bufferBack.data(), m_bufferBack.size());
    }
}

// Calculates samples required, requests
// the number from RAMAN, and then
// writes to output device using samplerate
// API.
double EngineBufferScaleSR::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    if (m_dBaseRate == 0.0 || m_dTempoRatio == 0.0 || !m_pResampler) {
        SampleUtil::clear(pOutputBuffer, iOutputBufferSize);
        return 0.0;
    }

    double readFramesProcessed = 0.0;
    double effectiveRate = m_dBaseRate * m_dTempoRatio;
    SINT remaining_frames = getOutputSignal().samples2frames(iOutputBufferSize);
    CSAMPLE* write_ptr = pOutputBuffer;

    while (remaining_frames > 0) {
        // Get input samples from ReadAheadManager
        SINT iAvailSamples = m_pReadAheadManager->getNextSamples(
                (m_bBackwards ? -1.0 : 1.0) * effectiveRate,
                m_bufferBack.data(),
                m_bufferBack.size(),
                getOutputSignal().getChannelCount());

        if (!iAvailSamples) {
            // No more input
            SampleUtil::clear(write_ptr, getOutputSignal().frames2samples(remaining_frames));
            break;
        }

        SRC_DATA src_data;
        src_data.data_in = m_bufferBack.data();
        src_data.input_frames = getOutputSignal().samples2frames(iAvailSamples);
        src_data.data_out = write_ptr;
        src_data.output_frames = remaining_frames;
        src_data.src_ratio = 1.0 / m_dTempoRatio;
        src_data.end_of_input = 0;

        int error = src_process(m_pResampler, &src_data);
        if (error) {
            qWarning() << "libsamplerate error:" << src_strerror(error);
            SampleUtil::clear(write_ptr, getOutputSignal().frames2samples(remaining_frames));
            break;
        }

        SINT frames_generated = src_data.output_frames_gen;
        readFramesProcessed += effectiveRate * src_data.input_frames_used;
        remaining_frames -= frames_generated;
        write_ptr += getOutputSignal().frames2samples(frames_generated);

        // If no output was generated, break to avoid infinite loop
        if (frames_generated == 0) {
            SampleUtil::clear(write_ptr, getOutputSignal().frames2samples(remaining_frames));
            break;
        }
    }
    return readFramesProcessed;
}

void EngineBufferScaleSR::onSignalChanged() {
    int backBufferSize = kBackBufferFrameSize * getOutputSignal().getChannelCount();
    if (m_bufferBack.size() == backBufferSize) {
        m_bufferBack.clear();
    } else {
        m_bufferBack = mixxx::SampleBuffer(backBufferSize);
    }

    if (!getOutputSignal().isValid()) {
        return;
    }

    clear();
}
