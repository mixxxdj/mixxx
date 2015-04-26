#include "sources/soundsourcesndfile.h"

namespace Mixxx {

QList<QString> SoundSourceSndFile::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("aiff");
    list.push_back("aif");
    list.push_back("wav");
    list.push_back("flac");
    return list;
}

SoundSourceSndFile::SoundSourceSndFile(QUrl url)
        : SoundSource(url),
          m_pSndFile(NULL) {
    memset(&m_sfInfo, 0, sizeof(m_sfInfo));
}

SoundSourceSndFile::~SoundSourceSndFile() {
    close();
}

Result SoundSourceSndFile::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    DEBUG_ASSERT(!m_pSndFile);
#ifdef __WINDOWS__
    // Pointer valid until string changed
    const QString fileName(getLocalFileName());
    LPCWSTR lpcwFilename = (LPCWSTR) fileName.utf16();
    m_pSndFile = sf_wchar_open(lpcwFilename, SFM_READ, &m_sfInfo);
#else
    m_pSndFile = sf_open(getLocalFileNameBytes().constData(), SFM_READ,
            &m_sfInfo);
#endif

    if (!m_pSndFile) {   // sf_format_check is only for writes
        qWarning() << "Error opening libsndfile file:" << getUrlString()
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    if (sf_error(m_pSndFile) > 0) {
        qWarning() << "Error opening libsndfile file:" << getUrlString()
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    setChannelCount(m_sfInfo.channels);
    setFrameRate(m_sfInfo.samplerate);
    setFrameCount(m_sfInfo.frames);

    return OK;
}

void SoundSourceSndFile::close() {
    if (m_pSndFile) {
        const int closeResult = sf_close(m_pSndFile);
        if (0 == closeResult) {
            m_pSndFile = NULL;
        } else {
            qWarning() << "Failed to close file:" << closeResult
                    << sf_strerror(m_pSndFile)
                    << getUrlString();
        }
    }
}

SINT SoundSourceSndFile::seekSampleFrame(
        SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    const sf_count_t seekResult = sf_seek(m_pSndFile, frameIndex, SEEK_SET);
    if (0 <= seekResult) {
        return seekResult;
    } else {
        qWarning() << "Failed to seek libsnd file:" << seekResult
                << sf_strerror(m_pSndFile);
        return sf_seek(m_pSndFile, 0, SEEK_CUR);
    }
}

SINT SoundSourceSndFile::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    const sf_count_t readCount =
            sf_readf_float(m_pSndFile, sampleBuffer, numberOfFrames);
    if (0 <= readCount) {
        return readCount;
    } else {
        qWarning() << "Failed to read from libsnd file:" << readCount
                << sf_strerror(m_pSndFile);
        return 0;
    }
}

} // namespace Mixxx
