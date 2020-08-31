#include "sources/soundsourcewv.h"

#include <wavpack/wavpack.h>

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceWV");

static WavpackStreamReader s_streamReader = {
        SoundSourceWV::ReadBytesCallback,
        SoundSourceWV::GetPosCallback,
        SoundSourceWV::SetPosAbsCallback,
        SoundSourceWV::SetPosRelCallback,
        SoundSourceWV::PushBackByteCallback,
        SoundSourceWV::GetlengthCallback,
        SoundSourceWV::CanSeekCallback,
        SoundSourceWV::WriteBytesCallback};

} // anonymous namespace

//static
const QString SoundSourceProviderWV::kDisplayName = QStringLiteral("WavPack");

//static
const QStringList SoundSourceProviderWV::kSupportedFileExtensions = {
        QStringLiteral("wv"),
};

SoundSourceProviderPriority SoundSourceProviderWV::getPriorityHint(
        const QString& supportedFileExtension) const {
    Q_UNUSED(supportedFileExtension)
    // This reference decoder is supposed to produce more accurate
    // and reliable results than any other DEFAULT provider.
    return SoundSourceProviderPriority::Higher;
}

SoundSourcePointer SoundSourceProviderWV::newSoundSource(const QUrl& url) {
    return newSoundSourceFromUrl<SoundSourceWV>(url);
}

SoundSourceWV::SoundSourceWV(const QUrl& url)
        : SoundSource(url),
          m_wpc(nullptr),
          m_sampleScaleFactor(CSAMPLE_ZERO),
          m_pWVFile(nullptr),
          m_pWVCFile(nullptr),
          m_curFrameIndex(0) {
}

SoundSourceWV::~SoundSourceWV() {
    close();
}

SoundSource::OpenResult SoundSourceWV::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& params) {
    DEBUG_ASSERT(!m_wpc);
    char msg[80]; // hold possible error message
    int openFlags = OPEN_WVC | OPEN_NORMALIZE;
    if ((params.getSignalInfo().getChannelCount() == 1) ||
            (params.getSignalInfo().getChannelCount() == 2)) {
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
    m_wpc = WavpackOpenFileInputEx(&s_streamReader, m_pWVFile, m_pWVCFile, msg, openFlags, 0);
    if (!m_wpc) {
        kLogger.warning() << "failed to open file : " << msg;
        return OpenResult::Failed;
    }

    initChannelCountOnce(WavpackGetReducedChannels(static_cast<WavpackContext*>(m_wpc)));
    initSampleRateOnce(WavpackGetSampleRate(static_cast<WavpackContext*>(m_wpc)));
    initFrameIndexRangeOnce(
            mixxx::IndexRange::forward(
                    0,
                    WavpackGetNumSamples(static_cast<WavpackContext*>(m_wpc))));

    if (WavpackGetMode(static_cast<WavpackContext*>(m_wpc)) & MODE_FLOAT) {
        m_sampleScaleFactor = CSAMPLE_PEAK;
    } else {
        const int bitsPerSample = WavpackGetBitsPerSample(static_cast<WavpackContext*>(m_wpc));
        if ((bitsPerSample >= 8) && (bitsPerSample <= 32)) {
            // Range of signed sample values: [-2 ^ (bitsPerSample - 1), 2 ^ (bitsPerSample - 1) - 1]
            const uint32_t absSamplePeak = 1u << (bitsPerSample - 1);
            DEBUG_ASSERT(absSamplePeak > 0);
            // Scaled range of sample values: [-CSAMPLE_PEAK, CSAMPLE_PEAK)
            m_sampleScaleFactor = CSAMPLE_PEAK / absSamplePeak;
        } else {
            kLogger.warning()
                    << "Invalid bits per sample:"
                    << bitsPerSample;
            return OpenResult::Aborted;
        }
    }

    m_curFrameIndex = frameIndexMin();

    return OpenResult::Succeeded;
}

void SoundSourceWV::close() {
    if (m_wpc) {
        WavpackCloseFile(static_cast<WavpackContext*>(m_wpc));
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

ReadableSampleFrames SoundSourceWV::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (m_curFrameIndex != firstFrameIndex) {
        if (WavpackSeekSample(static_cast<WavpackContext*>(m_wpc), firstFrameIndex)) {
            m_curFrameIndex = firstFrameIndex;
        } else {
            kLogger.warning()
                    << "Could not seek to first frame index"
                    << firstFrameIndex;
            m_curFrameIndex = WavpackGetSampleIndex(static_cast<WavpackContext*>(m_wpc));
            return ReadableSampleFrames(IndexRange::between(m_curFrameIndex, m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfFramesTotal = writableSampleFrames.frameLength();

    static_assert(sizeof(CSAMPLE) == sizeof(int32_t),
            "CSAMPLE and int32_t must have the same size");
    CSAMPLE* pOutputBuffer = writableSampleFrames.writableData();
    SINT unpackCount = WavpackUnpackSamples(static_cast<WavpackContext*>(m_wpc),
            reinterpret_cast<int32_t*>(pOutputBuffer),
            numberOfFramesTotal);
    DEBUG_ASSERT(unpackCount >= 0);
    DEBUG_ASSERT(unpackCount <= numberOfFramesTotal);
    if (!(WavpackGetMode(static_cast<WavpackContext*>(m_wpc)) & MODE_FLOAT)) {
        // signed integer -> float
        const SINT sampleCount = getSignalInfo().frames2samples(unpackCount);
        for (SINT i = 0; i < sampleCount; ++i) {
            const int32_t sampleValue =
                    *reinterpret_cast<int32_t*>(pOutputBuffer);
            *pOutputBuffer++ = CSAMPLE(sampleValue) * m_sampleScaleFactor;
        }
    }
    const auto resultRange = IndexRange::forward(m_curFrameIndex, unpackCount);
    m_curFrameIndex += unpackCount;
    return ReadableSampleFrames(
            resultRange,
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    getSignalInfo().frames2samples(unpackCount)));
}

//static
int32_t SoundSourceWV::ReadBytesCallback(void* id, void* data, int bcount) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->read((char*)data, bcount);
}

// static
uint32_t SoundSourceWV::GetPosCallback(void* id) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->pos();
}

//static
int SoundSourceWV::SetPosAbsCallback(void* id, unsigned int pos) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->seek(pos) ? 0 : -1;
}

//static
int SoundSourceWV::SetPosRelCallback(void* id, int delta, int mode) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }

    switch (mode) {
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
int SoundSourceWV::PushBackByteCallback(void* id, int c) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    pFile->ungetChar((char)c);
    return 1;
}

//static
uint32_t SoundSourceWV::GetlengthCallback(void* id) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->size();
}

//static
int SoundSourceWV::CanSeekCallback(void* id) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return pFile->isSequential() ? 0 : 1;
}

//static
int32_t SoundSourceWV::WriteBytesCallback(void* id, void* data, int32_t bcount) {
    QFile* pFile = static_cast<QFile*>(id);
    if (!pFile) {
        return 0;
    }
    return (int32_t)pFile->write((char*)data, bcount);
}

} // namespace mixxx
