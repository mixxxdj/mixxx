#include <QDir>

#include "sources/soundsourcesndfile.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceSndFile");

} // anonymous namespace

SoundSourceSndFile::SoundSourceSndFile(const QUrl& url)
        : SoundSource(url),
          m_pSndFile(nullptr),
          m_curFrameIndex(0) {
}

SoundSourceSndFile::~SoundSourceSndFile() {
    close();
}

SoundSource::OpenResult SoundSourceSndFile::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    DEBUG_ASSERT(!m_pSndFile);
    SF_INFO sfInfo;
    memset(&sfInfo, 0, sizeof(sfInfo));
#ifdef __WINDOWS__
    // Note: we cannot use QString::toStdWString since QT 4 is compiled with
    // '/Zc:wchar_t-' flag and QT 5 not
    const QString localFileName(QDir::toNativeSeparators(getLocalFileName()));
    const ushort* const fileNameUtf16 = localFileName.utf16();
    static_assert(sizeof(wchar_t) == sizeof(ushort), "QString::utf16(): wchar_t and ushort have different sizes");
    m_pSndFile = sf_wchar_open(
            reinterpret_cast<wchar_t*>(const_cast<ushort*>(fileNameUtf16)),
            SFM_READ,
            &sfInfo);
#else
    m_pSndFile = sf_open(QFile::encodeName(getLocalFileName()), SFM_READ, &sfInfo);
#endif

    switch (sf_error(m_pSndFile)) {
    case SF_ERR_NO_ERROR:
        DEBUG_ASSERT(m_pSndFile != nullptr);
        break; // continue
    case SF_ERR_UNRECOGNISED_FORMAT:
        return OpenResult::Aborted;
    default:
        const QString errorMsg(sf_strerror(m_pSndFile));
        if (errorMsg.toLower().indexOf("unknown format") != -1) {
            // NOTE(uklotzde 2016-05-11): This actually happens when
            // trying to open a file with a supported file extension
            // that contains data in an unsupported format!
            return OpenResult::Aborted;
        } else {
            kLogger.warning() << "Error opening libsndfile file:"
                              << getUrlString()
                              << errorMsg;
            return OpenResult::Failed;
        }
    }

    setChannelCount(sfInfo.channels);
    setSampleRate(sfInfo.samplerate);
    initFrameIndexRangeOnce(IndexRange::forward(0, sfInfo.frames));

    m_curFrameIndex = frameIndexMin();

    return OpenResult::Succeeded;
}

void SoundSourceSndFile::close() {
    if (m_pSndFile != nullptr) {
        const int closeResult = sf_close(m_pSndFile);
        if (0 == closeResult) {
            m_pSndFile = nullptr;
            m_curFrameIndex = frameIndexMin();
        } else {
            kLogger.warning() << "Failed to close file:" << closeResult
                              << sf_strerror(m_pSndFile)
                              << getUrlString();
        }
    }
}

ReadableSampleFrames SoundSourceSndFile::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (m_curFrameIndex != firstFrameIndex) {
        const sf_count_t seekResult = sf_seek(m_pSndFile, firstFrameIndex, SEEK_SET);
        if (seekResult == firstFrameIndex) {
            m_curFrameIndex = seekResult;
        } else {
            kLogger.warning() << "Failed to seek libsnd file:" << seekResult
                              << sf_strerror(m_pSndFile);
            m_curFrameIndex = sf_seek(m_pSndFile, 0, SEEK_CUR);
            return ReadableSampleFrames(IndexRange::between(m_curFrameIndex, m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfFramesTotal = writableSampleFrames.frameLength();

    const sf_count_t readCount =
            sf_readf_float(m_pSndFile, writableSampleFrames.writableData(), numberOfFramesTotal);
    if (readCount >= 0) {
        DEBUG_ASSERT(readCount <= numberOfFramesTotal);
        const auto resultRange = IndexRange::forward(m_curFrameIndex, readCount);
        m_curFrameIndex += readCount;
        return ReadableSampleFrames(
                resultRange,
                SampleBuffer::ReadableSlice(
                        writableSampleFrames.writableData(),
                        frames2samples(readCount)));
    } else {
        kLogger.warning() << "Failed to read from libsnd file:"
                          << readCount
                          << sf_strerror(m_pSndFile);
        return ReadableSampleFrames(
                IndexRange::between(
                        m_curFrameIndex,
                        m_curFrameIndex));
    }
}

QString SoundSourceProviderSndFile::getName() const {
    return "libsndfile";
}

QStringList SoundSourceProviderSndFile::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("aif");
    supportedFileExtensions.append("aiff");
    // ALAC/CAF has been added in version 1.0.26
    // NOTE(uklotzde, 2015-05-26): Unfortunately ALAC in M4A containers
    // is still not supported https://github.com/mixxxdj/mixxx/pull/904#issuecomment-221928362
    supportedFileExtensions.append("caf");
    supportedFileExtensions.append("flac");
    supportedFileExtensions.append("ogg");
    supportedFileExtensions.append("wav");
    return supportedFileExtensions;
}

SoundSourceProviderPriority SoundSourceProviderSndFile::getPriorityHint(
        const QString& supportedFileExtension) const {
    if (supportedFileExtension.startsWith("aif") ||
            supportedFileExtension == "wav") {
        // Default decoder for AIFF and WAV
        return SoundSourceProviderPriority::DEFAULT;
    } else {
        // Otherwise only used as fallback
        return SoundSourceProviderPriority::LOWER;
    }
}

} // namespace mixxx
