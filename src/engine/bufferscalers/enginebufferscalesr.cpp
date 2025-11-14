#include "engine/bufferscalers/enginebufferscalesr.h"

extern "C" {
#include "engine/bufferscalers/libsamplerate-common.h"
}

#include <QDebug>

#include "engine/enginebuffer.h"
#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesr.cpp"
#include "util/math.h"
#include "util/sample.h"

namespace {
constexpr SINT kBackBufferFrameSize = 512;     // configurable
constexpr double kMinValidRatio = 1.0 / 256.0; // Minimum valid sample rate ratio
constexpr double kMaxValidRatio = 256.0;       // Maximum valid sample rate ratio

// Callback API requires a callback
// to retrieve input audio and make it accessible
// to the resampler. Called by src_callback_read()
// during playback.
// ---
// For mixxx, this function should invoke RAMAN
// cb_data->buffer must contain the data from RAMAN,
// and the audio pointer must be updated to point to
// that data. This makes track samples available to the
// converter for scaling.
// data: SRC_CB_DATA
// ---
// Needs to be a static function, since it is called by libsamplerate core.
// definition in namespace == static declaration.
long raman_get_input_frames_cb(void* pCb_data, float** ppAudio) {
    // qDebug() << "callback: get input from RAMAN";

    auto* pThis = static_cast<EngineBufferScaleSR*>(pCb_data);

    return pThis->getInputFrames(ppAudio); // public
}

} // namespace

// XXX DO NOT MERGE WITH UPSTREAM
double src_get_last_position(SRC_STATE* state) {
    return state->last_position;
}

EngineBufferScaleSR::EngineBufferScaleSR(ReadAheadManager* pReadAheadManager, double e_Index)
        : m_pReadAheadManager(pReadAheadManager),
          m_bBackwards(false),
          m_pResampler(nullptr) {
    m_effectiveRate = 1.0;
    m_inputFramesRead = 0.0;
    m_dQuality = SRC_SINC_FASTEST;
    m_lastPositionOld = 0.0;
    m_savedFramesOld = 0.0;
    setQuality(e_Index);
}

EngineBufferScaleSR::~EngineBufferScaleSR() {
    if (m_pResampler) {
        src_delete(m_pResampler);
        m_pResampler = nullptr;
    }
}

void EngineBufferScaleSR::setQuality(double engine_quality) {
    if (m_pResampler) {
        src_delete(m_pResampler);
        m_pResampler = nullptr;
    }

    auto quality = static_cast<EngineBuffer::ScratchingEngine>(engine_quality);
    m_dChannels = getOutputSignal().getChannelCount();

    int error = 0;

    switch (quality) {
    case EngineBuffer::ScratchingEngine::SampleRateLinear:
        m_dQuality = SRC_LINEAR;
        break;
    case EngineBuffer::ScratchingEngine::SampleRateSincFastest:
        m_dQuality = SRC_SINC_FASTEST;
        break;
    case EngineBuffer::ScratchingEngine::SampleRateSincFinest:
        m_dQuality = SRC_SINC_BEST_QUALITY;
        break;
    case EngineBuffer::ScratchingEngine::NaiveLinear:
        [[fallthrough]];
    default:
        DEBUG_ASSERT(!"Unsupported quality setting");
    }

    m_pResampler = src_callback_new(raman_get_input_frames_cb,
            m_dQuality,
            m_dChannels.value(),
            &error,
            this);

    if (error) {
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

// will keep being called by samplerate core until the required
// # input samples are retrieved. Occurs in a single call to src_callback_read()
long EngineBufferScaleSR::getInputFrames(float** ppAudio) {
    SINT samplesRequested = static_cast<SINT>(m_bufferBack.size());

    SINT samplesRead = m_pReadAheadManager->getNextSamples(
            m_effectiveRate,
            m_bufferBack.data(),
            samplesRequested,
            m_dChannels);

    if (!samplesRead) {
        *ppAudio = nullptr;
        return 0;
    }

    SINT framesRead = getOutputSignal().samples2frames(samplesRead);
    m_inputFramesRead += framesRead;

    *ppAudio = m_bufferBack.data();
    return static_cast<long>(framesRead); // return frames to samplerate core.
}

// Generate input data via cb, then scale it and return maximum of
// `out_frames` frames. clearly, max out_frames is size of backbuffer.
// samples: ~backbuffer. Where RAMAN samples are cached before scale.
// ---
// Using a smaller backbuffer:
// samplerate core will perform a <read-input-from-callback ->src_process ->
// update pOutput offset> loop until out_frames frames are generated.
long EngineBufferScaleSR::do_scale(CSAMPLE* pOutput, SINT outFrames) {
    if (!m_pResampler) {
        qWarning() << "Resampler not initialized!";
        return 0;
    }

    double srcRatio = 1.0 / fabs(m_effectiveRate);
    srcRatio = std::clamp(srcRatio, kMinValidRatio, kMaxValidRatio);
    qDebug() << "src ratio: " << srcRatio;

    double lastPositionBefore = src_get_last_position(m_pResampler);
    // double inputFramesReadBefore = m_inputFramesRead;

    // not exactly correct. We need to track frames consumed by samplerate core,
    // every DAC callback, but the samplerate callback API does not allow
    // access to saved_frames.
    double expectedinputFramesRead = outFrames / srcRatio; // theoretical value
    qDebug() << "Expect " << expectedinputFramesRead << " frames from RAMAN.";

    // calls RAMAN for input
    // approx. required_input_frames frames will be consumed
    // over several read callbacks.
    long framesGenerated = src_callback_read(
            m_pResampler,
            srcRatio,
            outFrames,
            pOutput);

    // Calculate fractional advancement (handle wrap-around)
    double fractionalAdvancement = src_get_last_position(m_pResampler) - lastPositionBefore;
    if (fractionalAdvancement < 0) {
        fractionalAdvancement += 1.0;
    }
    qDebug() << "Fractional advancement: " << fractionalAdvancement;

    m_inputFramesRead = (m_inputFramesRead + m_savedFramesOld -
                                m_pResampler->saved_frames) +
            fractionalAdvancement;

    return framesGenerated;
}

// Performs an upsample or downsample depending on
// the effective rate.
// Returns #frames of input track consumed
double EngineBufferScaleSR::scaleBuffer(
        CSAMPLE* pOutputBuffer,
        SINT iOutputBufferSize) {
    if (!pOutputBuffer || iOutputBufferSize <= 0 || !m_pResampler ||
            m_dBaseRate == 0.0 || m_dTempoRatio == 0.0) {
        SampleUtil::clear(pOutputBuffer, iOutputBufferSize);
        return 0.0;
    }

    m_inputFramesRead = 0.0; // unused frames at end of previous callback.

    SINT outFrames = getOutputSignal().samples2frames(iOutputBufferSize);

    const long framesProduced = do_scale(pOutputBuffer, outFrames);
    qDebug() << "Integer frames consumed: " << (m_inputFramesRead - m_pResampler->saved_frames);
    qDebug() << "Precise frames consumed: " << m_inputFramesRead;
    qDebug() << "Wrote " << framesProduced << " frames to DAC";
    m_savedFramesOld = m_pResampler->saved_frames;

    return m_inputFramesRead;
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

// reset backbuffer state if track changed.
// reset resampler state if track changed.
void EngineBufferScaleSR::onSignalChanged() {
    qDebug() << "Signal changed";
    int backBufferSize = kBackBufferFrameSize * getOutputSignal().getChannelCount();

    if (m_bufferBack.size() != backBufferSize) {
        m_bufferBack = mixxx::SampleBuffer(backBufferSize); // num samples
    }

    if (!getOutputSignal().isValid()) {
        return;
    }

    clear();
}
