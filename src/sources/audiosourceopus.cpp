#include "sources/audiosourceopus.h"

namespace Mixxx {

AudioSourceOpus::AudioSourceOpus(QUrl url)
        : AudioSource(url),
          m_pOggOpusFile(NULL) {
}

AudioSourceOpus::~AudioSourceOpus() {
    close();
}

AudioSourcePointer AudioSourceOpus::create(QUrl url) {
    return onCreate(new AudioSourceOpus(url));
}

Result AudioSourceOpus::postConstruct() {
    const QString fileName(getUrl().toLocalFile());
    const QByteArray qbaFilename(fileName.toLocal8Bit());
    int errorCode = 0;
    m_pOggOpusFile = op_open_file(qbaFilename.constData(), &errorCode);
    if (!m_pOggOpusFile) {
        qDebug() << "Failed to open OggOpus file:" << fileName << "errorCode"
                << errorCode;
        return ERR;
    }

    if (!op_seekable(m_pOggOpusFile)) {
        qWarning() << "OggOpus file is not seekable:" << fileName;
        return ERR;
    }

    setChannelCount(op_channel_count(m_pOggOpusFile, -1));
    setFrameRate(kFrameRate);

    ogg_int64_t frameCount = op_pcm_total(m_pOggOpusFile, -1);
    if (0 <= frameCount) {
        setFrameCount(frameCount);
    } else {
        qWarning() << "Failed to read OggOpus file:" << fileName;
        return ERR;
    }

    return OK;
}

void AudioSourceOpus::close() {
    if (m_pOggOpusFile) {
        op_free(m_pOggOpusFile);
        m_pOggOpusFile = NULL;
    }
    reset();
}

AudioSource::diff_type AudioSourceOpus::seekSampleFrame(diff_type frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    int seekResult = op_pcm_seek(m_pOggOpusFile, frameIndex);
    if (0 != seekResult) {
        qWarning() << "Failed to seek OggOpus file:" << seekResult;
    }

    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));
    return getCurrentFrameIndex();
}

AudioSource::size_type AudioSourceOpus::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));

    const size_type numberOfFramesTotal = math_min(numberOfFrames,
            size_type(getFrameIndexMax() - getCurrentFrameIndex()));

    sample_type* pSampleBuffer = sampleBuffer;
    size_type numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        int readResult = op_read_float(m_pOggOpusFile,
                pSampleBuffer,
                frames2samples(numberOfFramesRemaining), NULL);
        if (0 == readResult) {
            // EOF
            break;// done
        }
        if (0 < readResult) {
            pSampleBuffer += frames2samples(readResult);
            numberOfFramesRemaining -= readResult;
        } else {
            qWarning() << "Failed to read sample data from OggOpus file:"
                    << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

AudioSource::size_type AudioSourceOpus::readSampleFramesStereo(
        size_type numberOfFrames, sample_type* sampleBuffer,
        size_type sampleBufferSize) {
    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, true) >= sampleBufferSize);

    const size_type numberOfFramesTotal = math_min(numberOfFrames,
            size_type(getFrameIndexMax() - getCurrentFrameIndex()));

    sample_type* pSampleBuffer = sampleBuffer;
    size_type numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        int readResult = op_read_float_stereo(m_pOggOpusFile,
                pSampleBuffer,
                numberOfFramesRemaining * 2);
        if (0 == readResult) {
            // EOF
            break;// done
        }
        if (0 < readResult) {
            pSampleBuffer += readResult * 2;
            numberOfFramesRemaining -= readResult;
        } else {
            qWarning() << "Failed to read sample data from OggOpus file:"
                    << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

} // namespace Mixxx
