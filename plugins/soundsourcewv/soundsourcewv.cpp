#include "soundsourcewv.h"

#include <QFile>

namespace mixxx {

//static
WavpackStreamReader SoundSourceWV::s_streamReader = {
    SoundSourceWV::ReadBytesCallback,
    SoundSourceWV::GetPosCallback,
    SoundSourceWV::SetPosAbsCallback,
    SoundSourceWV::SetPosRelCallback,
    SoundSourceWV::PushBackByteCallback,
    SoundSourceWV::GetlengthCallback,
    SoundSourceWV::CanSeekCallback,
    SoundSourceWV::WriteBytesCallback
};

SoundSourceWV::SoundSourceWV(const QUrl& url)
        : SoundSourcePlugin(url, "wv"),
          m_wpc(nullptr),
          m_sampleScaleFactor(CSAMPLE_ZERO), 
          m_pWVFile(nullptr),
          m_pWVCFile(nullptr) {
}

SoundSourceWV::~SoundSourceWV() {
    close();
}

SoundSource::OpenResult SoundSourceWV::tryOpen(const AudioSourceConfig& audioSrcCfg) {
    DEBUG_ASSERT(!m_wpc);
    char msg[80]; // hold possible error message
    int openFlags = OPEN_WVC | OPEN_NORMALIZE;
    if ((kChannelCountMono == audioSrcCfg.getChannelCount()) ||
            (kChannelCountStereo == audioSrcCfg.getChannelCount())) {
        openFlags |= OPEN_2CH_MAX;
    }

    // We use WavpackOpenFileInputEx to support Unicode paths on windows
    // http://www.wavpack.com/lib_use.txt
    QString wavPackFileName = getLocalFileName();
    m_pWVFile = new QFile(wavPackFileName);
    m_pWVFile->open(QFile::ReadOnly);
    QString correctionFileName(wavPackFileName + "c");
    if (QFile::exists(correctionFileName)) {
        // If there is a correction file, open it as well
        m_pWVCFile = new QFile(correctionFileName);
        m_pWVCFile->open(QFile::ReadOnly);
    }
    m_wpc = WavpackOpenFileInputEx(&s_streamReader, m_pWVFile, m_pWVCFile,
            msg, openFlags, 0);
    if (!m_wpc) {
        qDebug() << "SSWV::open: failed to open file : " << msg;
        return OpenResult::FAILED;
    }

    setChannelCount(WavpackGetReducedChannels(m_wpc));
    setSamplingRate(WavpackGetSampleRate(m_wpc));
    setFrameCount(WavpackGetNumSamples(m_wpc));

    if (WavpackGetMode(m_wpc) & MODE_FLOAT) {
        m_sampleScaleFactor = CSAMPLE_PEAK;
    } else {
        const int bitsPerSample = WavpackGetBitsPerSample(m_wpc);
        const uint32_t wavpackPeakSampleValue = 1u
                << (bitsPerSample - 1);
        m_sampleScaleFactor = CSAMPLE_PEAK / wavpackPeakSampleValue;
    }

    return OpenResult::SUCCEEDED;
}

void SoundSourceWV::close() {
    if (m_wpc) {
        WavpackCloseFile(m_wpc);
        m_wpc = nullptr;
    }
    if (m_pWVFile) {
        m_pWVFile->close();
        delete m_pWVFile;
        m_pWVFile = nullptr;
    }
    if (m_pWVCFile) {
        m_pWVCFile->close();
        delete m_pWVCFile;
        m_pWVCFile = nullptr;
    }
}

SINT SoundSourceWV::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));
    if (WavpackSeekSample(m_wpc, frameIndex) == true) {
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

//static
int32_t SoundSourceWV::ReadBytesCallback(void* id, void* data, int bcount)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->read((char*)data, bcount);
}


// static
uint32_t SoundSourceWV::GetPosCallback(void *id)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->pos();
}

//static
int SoundSourceWV::SetPosAbsCallback(void* id, unsigned int pos)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->seek(pos) ? 0 : -1;
}

//static
int SoundSourceWV::SetPosRelCallback(void *id, int delta, int mode)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }

    switch(mode) {
    case SEEK_SET:
        return pFile->seek(delta) ? 0 : -1;
    case SEEK_CUR:
        return pFile->seek(pFile->pos() + delta) ? 0 : -1;
    case SEEK_END:
        return pFile->seek(pFile->size() + delta) ? 0 : -1;
    default:
        return -1;
    }
}

//static
int SoundSourceWV::PushBackByteCallback(void* id, int c)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    pFile->ungetChar((char)c);
    return 1;
}

//static
uint32_t SoundSourceWV::GetlengthCallback(void* id)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->size();
}

//static
int SoundSourceWV::CanSeekCallback(void *id)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->isSequential() ? 0 : 1;
}

//static
int32_t SoundSourceWV::WriteBytesCallback(void* id, void* data, int32_t bcount)
{
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return (int32_t)pFile->write((char*)data, bcount);
}

} // namespace mixxx

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider() {
    // SoundSourceProviderWV is stateless and a single instance
    // can safely be shared
    static mixxx::SoundSourceProviderWV singleton;
    return &singleton;
}

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(mixxx::SoundSourceProvider*) {
    // The statically allocated instance must not be deleted!
}
