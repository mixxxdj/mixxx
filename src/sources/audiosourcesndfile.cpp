#include "sources/audiosourcesndfile.h"

namespace Mixxx
{


AudioSourceSndFile::AudioSourceSndFile()
        : m_pSndFile(NULL) {
    memset(&m_sfInfo, 0, sizeof(m_sfInfo));
}

AudioSourceSndFile::~AudioSourceSndFile() {
    close();
}

AudioSourcePointer AudioSourceSndFile::open(QString fileName) {
    AudioSourceSndFile* pAudioSourceSndFile(new AudioSourceSndFile);
    AudioSourcePointer pAudioSource(pAudioSourceSndFile); // take ownership
    if (OK == pAudioSourceSndFile->postConstruct(fileName)) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

Result AudioSourceSndFile::postConstruct(QString fileName) {
#ifdef __WINDOWS__
    // Pointer valid until string changed
    LPCWSTR lpcwFilename = (LPCWSTR)fileName.utf16();
    m_pSndFile = sf_wchar_open(lpcwFilename, SFM_READ, &m_sfInfo);
#else
    m_pSndFile = sf_open(fileName.toLocal8Bit().constData(), SFM_READ, &m_sfInfo);
#endif

    if (m_pSndFile == NULL) {   // sf_format_check is only for writes
        qWarning() << "Error opening libsndfile file:" << fileName
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    if (sf_error(m_pSndFile) > 0) {
        qWarning() << "Error opening libsndfile file:" << fileName
                << sf_strerror(m_pSndFile);
        return ERR;
    }

    setChannelCount(m_sfInfo.channels);
    setFrameRate(m_sfInfo.samplerate);
    setFrameCount(m_sfInfo.frames);

    return OK;
}

void AudioSourceSndFile::close() throw() {
    if (m_pSndFile) {
        const int closeResult = sf_close(m_pSndFile);
        if (0 != closeResult) {
            qWarning() << "Failed to close libsnd file:" << closeResult
                    << sf_strerror(m_pSndFile);
        }
        m_pSndFile = NULL;
        memset(&m_sfInfo, 0, sizeof(m_sfInfo));
    }
    reset();
}

AudioSource::diff_type AudioSourceSndFile::seekFrame(
        diff_type frameIndex) {
    const sf_count_t seekResult = sf_seek(m_pSndFile, frameIndex, SEEK_SET);
    if (0 <= seekResult) {
        return seekResult;
    } else {
        qWarning() << "Failed to seek libsnd file:" << seekResult
                << sf_strerror(m_pSndFile);
        return sf_seek(m_pSndFile, 0, SEEK_CUR);
    }
}

AudioSource::size_type AudioSourceSndFile::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    const sf_count_t readCount = sf_readf_float(m_pSndFile, sampleBuffer, frameCount);
    if (0 <= readCount) {
        return readCount;
    } else {
        qWarning() << "Failed to read from libsnd file:"
                << readCount << sf_strerror(m_pSndFile);
        return 0;
    }
}

} // namespace Mixxx
