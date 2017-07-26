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
        return OpenResult::ABORTED;
    default:
        const QString errorMsg(sf_strerror(m_pSndFile));
        if (errorMsg.toLower().indexOf("unknown format") != -1) {
            // NOTE(uklotzde 2016-05-11): This actually happens when
            // trying to open a file with a supported file extension
            // that contains data in an unsupported format!
            return OpenResult::ABORTED;
        } else {
            kLogger.warning() << "Error opening libsndfile file:"
                    << getUrlString()
                    << errorMsg;
            return OpenResult::FAILED;
        }
    }

    setChannelCount(sfInfo.channels);
    setSamplingRate(sfInfo.samplerate);
    initFrameIndexRange(mixxx::IndexRange::forward(0, sfInfo.frames));

    m_curFrameIndex = frameIndexMin();

    return OpenResult::SUCCEEDED;
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

IndexRange SoundSourceSndFile::readOrSkipSampleFrames(
        IndexRange frameIndexRange,
        SampleBuffer::WritableSlice* pOutputBuffer) {
    auto readableFrames =
            adjustReadableFrameIndexRangeAndOutputBuffer(
                    frameIndexRange, pOutputBuffer);
    if (readableFrames.empty()) {
        return readableFrames;
    }

    if (pOutputBuffer) {
        if (m_curFrameIndex != readableFrames.start()) {
            const sf_count_t seekResult = sf_seek(m_pSndFile, readableFrames.start(), SEEK_SET);
            if (seekResult == readableFrames.start()) {
                m_curFrameIndex = seekResult;
            } else {
                kLogger.warning() << "Failed to seek libsnd file:" << seekResult
                        << sf_strerror(m_pSndFile);
                m_curFrameIndex = sf_seek(m_pSndFile, 0, SEEK_CUR);
                return IndexRange::between(m_curFrameIndex, m_curFrameIndex);
            }
        }
    } else {
        // NOTE(uklotzde): The libsndfile API does not provide any
        // functions for skipping samples in the audio stream. Calling
        // API functions with a nullptr buffer does not return. Since
        // we don't want to read samples into a temporary buffer that
        // has to be allocated we are seeking to the position after
        // the skipped samples.
        if (m_curFrameIndex != readableFrames.end()) {
            const sf_count_t seekResult = sf_seek(m_pSndFile, readableFrames.end(), SEEK_SET);
            if (seekResult >= 0) {
                DEBUG_ASSERT(seekResult >= readableFrames.start());
                m_curFrameIndex = seekResult;
                return IndexRange::between(readableFrames.start(), seekResult);
            } else {
                kLogger.warning() << "Failed to seek libsnd file:" << seekResult
                        << sf_strerror(m_pSndFile);
                m_curFrameIndex = sf_seek(m_pSndFile, 0, SEEK_CUR);
                return IndexRange::between(m_curFrameIndex, m_curFrameIndex);
            }
        }
    }

    DEBUG_ASSERT(m_curFrameIndex == readableFrames.start());
    DEBUG_ASSERT(pOutputBuffer);
    const sf_count_t readCount =
            sf_readf_float(m_pSndFile, pOutputBuffer->data(), readableFrames.length());
    if (readCount >= 0) {
        DEBUG_ASSERT(readCount <= readableFrames.length());
        const auto result = IndexRange::forward(m_curFrameIndex, readCount);
        m_curFrameIndex += readCount;
        return result;
    } else {
        kLogger.warning() << "Failed to read from libsnd file:"
                << readCount
                << sf_strerror(m_pSndFile);
        return IndexRange();
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
