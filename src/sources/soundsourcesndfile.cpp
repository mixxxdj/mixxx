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
          m_curFrameIndex(getMinFrameIndex()) {
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
    setFrameCount(sfInfo.frames);

    m_curFrameIndex = getMinFrameIndex();

    return OpenResult::SUCCEEDED;
}

void SoundSourceSndFile::close() {
    if (m_pSndFile != nullptr) {
        const int closeResult = sf_close(m_pSndFile);
        if (0 == closeResult) {
            m_pSndFile = nullptr;
            m_curFrameIndex = getMinFrameIndex();
        } else {
            kLogger.warning() << "Failed to close file:" << closeResult
                    << sf_strerror(m_pSndFile)
                    << getUrlString();
        }
    }
}

SINT SoundSourceSndFile::seekSampleFrame(
        SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    if (frameIndex >= getMaxFrameIndex()) {
        // EOF
        m_curFrameIndex = getMaxFrameIndex();
        return m_curFrameIndex;
    }

    if (frameIndex == m_curFrameIndex) {
        return m_curFrameIndex;
    }

    const sf_count_t seekResult = sf_seek(m_pSndFile, frameIndex, SEEK_SET);
    if (0 <= seekResult) {
        m_curFrameIndex = seekResult;
        return seekResult;
    } else {
        kLogger.warning() << "Failed to seek libsnd file:" << seekResult
                << sf_strerror(m_pSndFile);
        return sf_seek(m_pSndFile, 0, SEEK_CUR);
    }
}

SINT SoundSourceSndFile::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    VERIFY_OR_DEBUG_ASSERT(numberOfFrames >= 0) {
        return 0;
    }
    if (numberOfFrames == 0) {
        return 0;
    }
    if (sampleBuffer == nullptr) {
        // NOTE(uklotzde): The libsndfile API does not provide any
        // functions for skipping samples in the audio stream. Calling
        // API functions with a nullptr buffer does not return. Since
        // we don't want to read samples into a temporary buffer that
        // has to be allocated we are seeking to the position after
        // the skipped samples.
        SINT curFrameIndexBefore = m_curFrameIndex;
        SINT curFrameIndexAfter = seekSampleFrame(m_curFrameIndex + numberOfFrames);
        DEBUG_ASSERT(curFrameIndexBefore <= curFrameIndexAfter);
        DEBUG_ASSERT(m_curFrameIndex == curFrameIndexAfter);
        return curFrameIndexAfter - curFrameIndexBefore;
    }
    const sf_count_t readCount =
            sf_readf_float(m_pSndFile, sampleBuffer, numberOfFrames);
    if (0 <= readCount) {
        m_curFrameIndex += readCount;
        return readCount;
    } else {
        kLogger.warning() << "Failed to read from libsnd file:" << readCount
                << sf_strerror(m_pSndFile);
        return 0;
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
