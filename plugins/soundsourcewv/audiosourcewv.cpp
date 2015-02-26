#include "audiosourcewv.h"

namespace Mixxx {

AudioSourceWV::AudioSourceWV(QUrl url)
        : AudioSource(url),
          m_wpc(NULL),
          m_sampleScale(kSampleValueZero) {
}

AudioSourceWV::~AudioSourceWV() {
    preDestroy();
}

AudioSourcePointer AudioSourceWV::create(QUrl url) {
    return onCreate(new AudioSourceWV(url));
}

Result AudioSourceWV::postConstruct() {
    char msg[80]; // hold possible error message
    m_wpc = WavpackOpenFileInput(
            getLocalFileNameBytes().constData(), msg,
            OPEN_2CH_MAX | OPEN_WVC | OPEN_NORMALIZE, 0);
    if (!m_wpc) {
        qDebug() << "SSWV::open: failed to open file : " << msg;
        return ERR;
    }

    setChannelCount(WavpackGetReducedChannels(m_wpc));
    setFrameRate(WavpackGetSampleRate(m_wpc));
    setFrameCount(WavpackGetNumSamples(m_wpc));

    if (WavpackGetMode(m_wpc) & MODE_FLOAT) {
        m_sampleScale = kSampleValuePeak;
    } else {
        const int bitsPerSample = WavpackGetBitsPerSample(m_wpc);
        const uint32_t wavpackPeakSampleValue = uint32_t(1)
                << (bitsPerSample - 1);
        m_sampleScale = kSampleValuePeak / CSAMPLE(wavpackPeakSampleValue);
    }

    return OK;
}

void AudioSourceWV::preDestroy() {
    if (m_wpc) {
        WavpackCloseFile(m_wpc);
        m_wpc = NULL;
    }
}

SINT AudioSourceWV::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));
    if (WavpackSeekSample(m_wpc, frameIndex) == TRUE) {
        return frameIndex;
    } else {
        qDebug() << "SSWV::seek : could not seek to frame #" << frameIndex;
        return WavpackGetSampleIndex(m_wpc);
    }
}

SINT AudioSourceWV::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    // static assert: sizeof(CSAMPLE) == sizeof(int32_t)
    SINT unpackCount = WavpackUnpackSamples(m_wpc,
            reinterpret_cast<int32_t*>(sampleBuffer), numberOfFrames);
    if (!(WavpackGetMode(m_wpc) & MODE_FLOAT)) {
        // signed integer -> float
        const SINT sampleCount = frames2samples(unpackCount);
        for (SINT i = 0; i < sampleCount; ++i) {
            const int32_t sampleValue =
                    reinterpret_cast<int32_t*>(sampleBuffer)[i];
            sampleBuffer[i] = CSAMPLE(sampleValue) * m_sampleScale;
        }
    }
    return unpackCount;
}

} // namespace Mixxx
