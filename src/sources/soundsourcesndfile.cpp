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

SoundSource::OpenResult SoundSourceSndFile::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
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
    m_pSndFile = sf_open(getLocalFileName().toLocal8Bit(), SFM_READ, &sfInfo);
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
    setSamplingRate(sfInfo.samplerate);
    initFrameIndexRangeOnce(mixxx::IndexRange::forward(0, sfInfo.frames));

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
        ReadMode readMode,
        WritableSampleFrames writableSampleFrames) {

    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (readMode == ReadMode::Skip) {
        // NOTE(uklotzde): The libsndfile API does not provide any
        // functions for skipping samples in the audio stream. Calling
        // API functions with a nullptr buffer does not return. Since
        // we don't want to read samples into a temporary buffer that
        // has to be allocated we are seeking to the position after
        // the skipped samples.
        if (m_curFrameIndex != writableSampleFrames.frameIndexRange().end()) {
            const sf_count_t seekResult = sf_seek(m_pSndFile, writableSampleFrames.frameIndexRange().end(), SEEK_SET);
            if (seekResult >= 0) {
                DEBUG_ASSERT(seekResult >= firstFrameIndex);
                m_curFrameIndex = seekResult;
                return ReadableSampleFrames(IndexRange::between(firstFrameIndex, seekResult));
            } else {
                kLogger.warning() << "Failed to seek libsnd file:" << seekResult
                        << sf_strerror(m_pSndFile);
                m_curFrameIndex = sf_seek(m_pSndFile, 0, SEEK_CUR);
                // Abort
                return ReadableSampleFrames(
                        IndexRange::between(
                                m_curFrameIndex,
                                m_curFrameIndex));
            }
        }
    } else {
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
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);
    DEBUG_ASSERT(readMode != ReadMode::Skip);

    const SINT numberOfFramesTotal = writableSampleFrames.frameIndexRange().length();

    const sf_count_t readCount =
            sf_readf_float(m_pSndFile, writableSampleFrames.sampleBuffer().data(), numberOfFramesTotal);
    if (readCount >= 0) {
        DEBUG_ASSERT(readCount <= numberOfFramesTotal);
        const auto resultRange = IndexRange::forward(m_curFrameIndex, readCount);
        m_curFrameIndex += readCount;
        return ReadableSampleFrames(
                resultRange,
                SampleBuffer::ReadableSlice(
                        writableSampleFrames.sampleBuffer().data(),
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
    supportedFileExtensions.append("aiff");
    supportedFileExtensions.append("aif");
    supportedFileExtensions.append("wav");
    supportedFileExtensions.append("flac");
    supportedFileExtensions.append("ogg");
    // ALAC/CAF has been added in version 1.0.26
    // NOTE(uklotzde, 2015-05-26): Unfortunately ALAC in M4A containers
    // is still not supported https://github.com/mixxxdj/mixxx/pull/904#issuecomment-221928362
    supportedFileExtensions.append("caf");
    return supportedFileExtensions;
}

} // namespace mixxx
