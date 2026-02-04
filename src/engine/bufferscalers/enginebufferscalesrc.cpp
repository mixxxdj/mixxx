/**
 * FILE: enginebufferscalesrc.cpp
 * ------------------------------
 * Class implementing real-time buffer scaling
 * using the libsamplerate (Secret Rabbit Code) API
 */

#include "engine/bufferscalers/enginebufferscalesrc.h"

#include <QDebug>

#include "engine/enginebuffer.h"
#include "engine/readaheadmanager.h"
#include "moc_enginebufferscalesrc.cpp"
#include "util/math.h"
#include "util/sample.h"

namespace {
constexpr int kMaxInterpolationFactor = 12;
} // namespace

EngineBufferScaleSRC::EngineBufferScaleSRC()
        : m_pResampler(nullptr) {
    int error{};
    m_dChannels = getOutputSignal().getChannelCount();
    m_pResampler = src_new(SRC_SINC_BEST_QUALITY, m_dChannels.value(), &error);
    if (error || !m_pResampler) {
        qWarning() << "libsamplerate initialization error:" << src_strerror(error);
        m_pResampler = nullptr;
    }

    clear();
}

EngineBufferScaleSRC::~EngineBufferScaleSRC() {
}

void EngineBufferScaleSRC::clear() {
    if (m_pResampler) {
        src_reset(m_pResampler);
    }
}

// baseRate: engine rate/recordingrate
double EngineBufferScaleSRC::recScaleBuffer(const CSAMPLE* pInputBuffer,
        CSAMPLE* pOutputBuffer,
        SINT iInputBufferSize,
        double srcRatio) {
    if (!pOutputBuffer || !pInputBuffer || iInputBufferSize <= 0) {
        return 0.0;
    }
    qDebug() << "input buffer size samples (from sidechain): " << iInputBufferSize;

    // create SRC_DATA struct
    SRC_DATA src_data;
    src_data.data_in = pInputBuffer;

    // frames that need to be resampled
    SINT numInputFrames = getOutputSignal().samples2frames(iInputBufferSize);
    src_data.input_frames = numInputFrames;
    src_data.data_out = pOutputBuffer;

    // max # output frames = (96khz/8khz) * #input frames
    src_data.output_frames = kMaxInterpolationFactor * numInputFrames;

    // conversion ratio
    src_data.src_ratio = srcRatio;
    src_data.end_of_input = 0;

    int error = src_process(m_pResampler, &src_data);
    if (error) {
        qWarning() << "libsamplerate error:" << src_strerror(error);
    }

    SINT frames_generated = src_data.output_frames_gen;
    qDebug() << "SRC consumed " << src_data.input_frames_used << " input frames, produced "
             << frames_generated << " output frames";

    return frames_generated;
}

double EngineBufferScaleSRC::scaleBuffer(CSAMPLE* /*pBuffer*/, SINT /*iBufferSize*/) {
    qWarning() << "Using temporary stub implementation";
    return 1.0;
}

void EngineBufferScaleSRC::onSignalChanged() {
    qWarning() << "Using temporary stub implementation";
}
