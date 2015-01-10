#include "audiosourcewv.h"

namespace Mixxx {

AudioSourceWV::AudioSourceWV()
        : m_wpc(NULL), m_sampleScale(0.0f) {
}

AudioSourceWV::~AudioSourceWV() {
    close();
}

AudioSourcePointer AudioSourceWV::create(QString fileName) {
    QSharedPointer<AudioSourceWV> pAudioSource(new AudioSourceWV);
    if (OK == pAudioSource->open(fileName)) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

Result AudioSourceWV::open(QString fileName) {
    char msg[80]; // hold possible error message
    m_wpc = WavpackOpenFileInput(fileName.toLocal8Bit().constData(), msg,
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
        const uint32_t peakSampleValue = uint32_t(1) << (bitsPerSample - 1);
        m_sampleScale = kSampleValuePeak / sample_type(peakSampleValue);
    }

    return OK;
}

void AudioSourceWV::close() {
    if (m_wpc) {
        WavpackCloseFile(m_wpc);
        m_wpc = NULL;
    }
    reset();
}

AudioSource::diff_type AudioSourceWV::seekSampleFrame(diff_type frameIndex) {
    if (WavpackSeekSample(m_wpc, frameIndex) == TRUE) {
        return frameIndex;
    } else {
        qDebug() << "SSWV::seek : could not seek to frame #" << frameIndex;
        return WavpackGetSampleIndex(m_wpc);
    }
}

AudioSource::size_type AudioSourceWV::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    // static assert: sizeof(sample_type) == sizeof(int32_t)
    size_type unpackCount = WavpackUnpackSamples(m_wpc,
            reinterpret_cast<int32_t*>(sampleBuffer), numberOfFrames);
    if (!(WavpackGetMode(m_wpc) & MODE_FLOAT)) {
        // signed integer -> float
        const size_type sampleCount = frames2samples(unpackCount);
        for (size_type i = 0; i < sampleCount; ++i) {
            const int32_t sampleValue =
                    reinterpret_cast<int32_t*>(sampleBuffer)[i];
            sampleBuffer[i] = sample_type(sampleValue) * m_sampleScale;
        }
    }
    return unpackCount;
}

} // namespace Mixxx
