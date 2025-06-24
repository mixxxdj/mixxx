#include "engine/bufferscalers/enginebufferscalesr.h"

#include <QDebug>

#include "engine/enginebuffer.h"
#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesr.cpp"
#include "util/math.h"
#include "util/sample.h"

namespace {
constexpr SINT kBackBufferFrameSize = 2048;
constexpr double kMinValidRatio = 1.0 / 256.0; // Minimum valid sample rate ratio
constexpr double kMaxValidRatio = 256.0;       // Maximum valid sample rate ratio
} // namespace

EngineBufferScaleSR::EngineBufferScaleSR(ReadAheadManager* pReadAheadManager, double eIndex)
        : m_pReadAheadManager(pReadAheadManager),
          m_bBackwards(false),
          m_pResampler(nullptr) {
    m_effectiveRate = 1.0;
    setQuality(eIndex);
}

EngineBufferScaleSR::~EngineBufferScaleSR() {
}

void EngineBufferScaleSR::setQuality(double engine_quality) {
    if (m_pResampler) {
        src_delete(m_pResampler);
        m_pResampler = nullptr;
    }

    auto quality = static_cast<EngineBuffer::ScratchingEngine>(engine_quality);
    m_dChannels = getOutputSignal().getChannelCount();

    int error{};
    if (quality == EngineBuffer::ScratchingEngine::SampleRateLinear) {
        m_pResampler = src_new(SRC_LINEAR, m_dChannels.value(), &error);
    } else if (quality == EngineBuffer::ScratchingEngine::SampleRateSincFinest) {
        m_pResampler = src_new(SRC_SINC_BEST_QUALITY, m_dChannels.value(), &error);
    } else if (quality == EngineBuffer::ScratchingEngine::SampleRateSincFastest) {
        m_pResampler = src_new(SRC_SINC_FASTEST, m_dChannels.value(), &error);
    }

    if (error || !m_pResampler) {
        qWarning() << "libsamplerate initialization error:" << src_strerror(error);
        m_pResampler = nullptr;
    }
}

// these parameters describe the "request" that
// needs to be handled by libsamplerate.
// use the default implementation for now
void EngineBufferScaleSR::setScaleParameters(double base_rate,
        double* pTempoRatio,
        double* pPitchRatio) {
    if (!pTempoRatio || !pPitchRatio) {
        qWarning() << "Null pointer passed to setScaleParameters";
        return;
    }

    // Negative speed means we are going backwards. pitch does not affect
    // the playback direction.
    qDebug() << "Setting scale parameters";
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

    // Calculate effective rate for use in scaleBuffer()
    m_effectiveRate = m_dBaseRate * m_dTempoRatio;

    qDebug() << "Scale parameters - Tempo:" << m_dTempoRatio
             << "Pitch:" << m_dPitchRatio
             << "Base rate:" << m_dBaseRate
             << "Effective rate:" << m_effectiveRate;
}

void EngineBufferScaleSR::clear() {
    if (m_pResampler) {
        src_reset(m_pResampler);
    }

    // Clear our input buffer
    if (m_bufferBack.size() > 0) {
        SampleUtil::clear(m_bufferBack.data(), m_bufferBack.size());
    }
}

// Performs an upsample or downsample depending on
// the effective rate.
// Returns #frames of input track consumed
double EngineBufferScaleSR::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize /*#interleaved samples required by DAC*/) {
    if (!pOutputBuffer || iOutputBufferSize <= 0) {
        return 0.0;
    }

    if (m_dBaseRate == 0.0 || m_dTempoRatio == 0.0 || !m_pResampler) {
        SampleUtil::clear(pOutputBuffer, iOutputBufferSize);
        return 0.0;
    }
    qDebug() << "Output buffer size: " << iOutputBufferSize;

    // if no scaling required
    if (fabs(m_effectiveRate) == 1) {
        // Direct copy without resampling
        SINT samples_to_read = iOutputBufferSize;
        SINT samples_read = m_pReadAheadManager->getNextSamples(
                m_effectiveRate,
                pOutputBuffer,
                samples_to_read,
                m_dChannels);

        // Clear any remaining samples if we didn't get enough
        if (samples_read < samples_to_read) {
            SampleUtil::clear(pOutputBuffer + samples_read, samples_to_read - samples_read);
        }

        return getOutputSignal().samples2frames(samples_read);
    }

    double readFramesProcessed = 0.0;
    CSAMPLE* write_ptr = pOutputBuffer;

    // libsamplerate expects inverse of effectiveRate
    double src_ratio = 1.0 / fabs(m_effectiveRate);
    src_ratio = std::max(kMinValidRatio, std::min(kMaxValidRatio, src_ratio));

    SINT remaining_frames = getOutputSignal().samples2frames(iOutputBufferSize);

    // We need to iteratively request the RAMAN for the #samples
    // required to be scaled. We store the samples in m_bufferBack
    // before scaling and writing to write_ptr
    // If the new #frames = m_effectiveRate * remaining_frames,
    // we can choose to read then scale <required_frames> frames at a time
    // via src_process until <m_effectiveRate * remaining_frames>
    // frames of the track have been processed.
    // ---
    while (remaining_frames > 0) {
        // magic number, src_ratio over m_effectiverate because +ve.
        SINT required_input_frames = static_cast<SINT>(std::ceil(remaining_frames / src_ratio + 2));
        SINT max_input_frames = getOutputSignal().samples2frames(m_bufferBack.size());

        // read backbuffersize or less samples at a time.
        required_input_frames = std::min(required_input_frames, max_input_frames);

        // Get input samples from ReadAheadManager
        SINT samples_requested = getOutputSignal().frames2samples(required_input_frames);
        SINT iAvailSamples = m_pReadAheadManager->getNextSamples(
                m_effectiveRate,     // base_rate * tempo ratio = 1/src_ratio
                m_bufferBack.data(), // write track samples here
                samples_requested,
                m_dChannels);

        if (!iAvailSamples) {
            break;
        }

        qDebug() << "Read " << iAvailSamples << " into backBuffer";
        // static_assert(samples2frames(iAvailSamples) == required_input_frames);

        SRC_DATA src_data;
        src_data.data_in = m_bufferBack.data();

        // frames that need to be resampled
        src_data.input_frames = getOutputSignal().samples2frames(iAvailSamples);
        src_data.data_out = write_ptr;

        // # frames required after resample
        src_data.output_frames = remaining_frames;

        // conversion ratio
        src_data.src_ratio = src_ratio;
        src_data.end_of_input = 0;

        int error = src_process(m_pResampler, &src_data);
        if (error) {
            qWarning() << "libsamplerate error:" << src_strerror(error);
            SampleUtil::clear(write_ptr, getOutputSignal().frames2samples(remaining_frames));
            break;
        }

        if (!src_data.output_frames_gen) {
            if (!src_data.input_frames_used) {
                qWarning() << "libsamplerate stalled - no progress made";
                break;
            }
            // libsamplerate consumed input but produced no output
            // not sure when this would happen
        }

        SINT frames_generated = src_data.output_frames_gen;

        // how many track frames were actually scaled.
        // Note that ideally, every RAMAN-returned frame must be scaled
        // i.e. we ensure that backbuffersize frames are scaled each time.
        // If < required_input_frames are scaled, the extra samples in the
        // backBuffer will be overwritten by readaheadmanager??
        // static_assert(required_input_frames == src_data.input_frames_used);
        readFramesProcessed += src_data.input_frames_used;
        remaining_frames -= frames_generated;
        write_ptr += getOutputSignal().frames2samples(frames_generated);

        qDebug() << "SRC consumed " << src_data.input_frames_used << " input frames, produced "
                 << frames_generated << " output frames, remaining: " << remaining_frames;
    }

    // Clear any remaining output buffer
    if (remaining_frames > 0) {
        SampleUtil::clear(write_ptr, getOutputSignal().frames2samples(remaining_frames));
    }

    return readFramesProcessed;
}

void EngineBufferScaleSR::onSignalChanged() {
    int backBufferSize = kBackBufferFrameSize * getOutputSignal().getChannelCount();

    if (m_bufferBack.size() != backBufferSize) {
        m_bufferBack = mixxx::SampleBuffer(backBufferSize);
    }

    if (!getOutputSignal().isValid()) {
        return;
    }

    clear();
}
