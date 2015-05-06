#include "soundsourcewv.h"

namespace Mixxx {

QList<QString> SoundSourceWV::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("wv");
    return list;
}

SoundSourceWV::SoundSourceWV(QUrl url)
        : SoundSourcePlugin(url, "wv"),
          m_wpc(NULL),
          m_sampleScaleFactor(CSAMPLE_ZERO) {
}

SoundSourceWV::~SoundSourceWV() {
    close();
}

Result SoundSourceWV::tryOpen(const AudioSourceConfig& audioSrcCfg) {
    DEBUG_ASSERT(!m_wpc);
    char msg[80]; // hold possible error message
    int openFlags = OPEN_WVC | OPEN_NORMALIZE;
    if ((1 == audioSrcCfg.channelCountHint) || (2 == audioSrcCfg.channelCountHint)) {
        // mono or stereo requested
        openFlags |= OPEN_2CH_MAX;
    }
    m_wpc = WavpackOpenFileInput(
            getLocalFileNameBytes().constData(), msg, openFlags, 0);
    if (!m_wpc) {
        qDebug() << "SSWV::open: failed to open file : " << msg;
        return ERR;
    }

    setChannelCount(WavpackGetReducedChannels(m_wpc));
    setFrameRate(WavpackGetSampleRate(m_wpc));
    setFrameCount(WavpackGetNumSamples(m_wpc));

    if (WavpackGetMode(m_wpc) & MODE_FLOAT) {
        m_sampleScaleFactor = CSAMPLE_PEAK;
    } else {
        const int bitsPerSample = WavpackGetBitsPerSample(m_wpc);
        const uint32_t wavpackPeakSampleValue = uint32_t(1)
                << (bitsPerSample - 1);
        m_sampleScaleFactor = CSAMPLE_PEAK / CSAMPLE(wavpackPeakSampleValue);
    }

    return OK;
}

void SoundSourceWV::close() {
    if (m_wpc) {
        WavpackCloseFile(m_wpc);
        m_wpc = NULL;
    }
}

SINT SoundSourceWV::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));
    if (WavpackSeekSample(m_wpc, frameIndex) == TRUE) {
        return frameIndex;
    } else {
        qDebug() << "SSWV::seek : could not seek to frame #" << frameIndex;
        return WavpackGetSampleIndex(m_wpc);
    }
}

SINT SoundSourceWV::readSampleFrames(
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
            sampleBuffer[i] = CSAMPLE(sampleValue) * m_sampleScaleFactor;
        }
    }
    return unpackCount;
}

}  // namespace Mixxx

extern "C" MY_EXPORT const char* getMixxxVersion() {
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion() {
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName) {
    return new Mixxx::SoundSourceWV(fileName);
}

extern "C" MY_EXPORT char** supportedFileExtensions() {
    const QList<QString> supportedFileExtensions(
            Mixxx::SoundSourceWV::supportedFileExtensions());
    return Mixxx::SoundSourcePlugin::allocFileExtensions(
            supportedFileExtensions);
}

extern "C" MY_EXPORT void freeFileExtensions(char** fileExtensions) {
    Mixxx::SoundSourcePlugin::freeFileExtensions(fileExtensions);
}
