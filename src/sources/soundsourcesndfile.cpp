#include <QDir>

#include "sources/soundsourcesndfile.h"

namespace Mixxx {

SoundSourceSndFile::SoundSourceSndFile(QUrl url)
        : SoundSource(url),
          m_pSndFile(NULL) {
}

SoundSourceSndFile::~SoundSourceSndFile() {
    close();
}

Result SoundSourceSndFile::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
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

    setChannelCount(sfInfo.channels);
    setSamplingRate(sfInfo.samplerate);
    setFrameCount(sfInfo.frames);

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

QString SoundSourceProviderSndFile::getName() const {
    return "libsndfile";
}

QStringList SoundSourceProviderSndFile::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("aiff");
    supportedFileExtensions.append("aif");
    supportedFileExtensions.append("wav");
    supportedFileExtensions.append("flac");
    return supportedFileExtensions;
}

} // namespace Mixxx
