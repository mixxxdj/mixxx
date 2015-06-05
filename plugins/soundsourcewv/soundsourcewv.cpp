#include "soundsourcewv.h"

namespace Mixxx {

SoundSourceWV::SoundSourceWV(const QUrl& url)
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
    if ((kChannelCountMono == audioSrcCfg.channelCountHint) ||
            (kChannelCountStereo == audioSrcCfg.channelCountHint)) {
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

QString SoundSourceProviderWV::getName() const {
    return "WavPack";
}

QStringList SoundSourceProviderWV::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("wv");
    return supportedFileExtensions;
}

SoundSourcePointer SoundSourceProviderWV::newSoundSource(const QUrl& url) {
    return exportSoundSourcePlugin(new SoundSourceWV(url));
}

} // namespace Mixxx

namespace {

void deleteSoundSourceProviderSingleton(Mixxx::SoundSourceProvider*) {
    // The statically allocated instance must not be deleted!
}

} // anonymous namespace

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
Mixxx::SoundSourceProviderPointer Mixxx_SoundSourcePluginAPI_getSoundSourceProvider() {
    // SoundSourceProviderWV is stateless and a single instance
    // can safely be shared
    static Mixxx::SoundSourceProviderWV singleton;
    return Mixxx::SoundSourceProviderPointer(
            &singleton,
            deleteSoundSourceProviderSingleton);
}
