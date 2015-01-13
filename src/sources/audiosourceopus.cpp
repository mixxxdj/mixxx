#include "sources/audiosourceopus.h"

namespace Mixxx
{

AudioSourceOpus::AudioSourceOpus()
        : m_pOggOpusFile(NULL) {
}

AudioSourceOpus::~AudioSourceOpus() {
    close();
}

AudioSourcePointer AudioSourceOpus::create(QString fileName) {
    QSharedPointer<AudioSourceOpus> pAudioSource(new AudioSourceOpus);
    if (OK == pAudioSource->open(fileName)) {
        // success
        return pAudioSource;
    } else {
        // failure
        return AudioSourcePointer();
    }
}

Result AudioSourceOpus::open(QString fileName) {

    int errorCode = 0;
    const QByteArray qbaFilename(fileName.toLocal8Bit());
    m_pOggOpusFile = op_open_file(qbaFilename.constData(), &errorCode);
    if (!m_pOggOpusFile) {
        qDebug()
            << "Failed to open OggOpus file:" << fileName
            << "errorCode" << errorCode;
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
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));
    int seekResult = op_pcm_seek(m_pOggOpusFile, frameIndex);
    if (0 != seekResult) {
        qWarning() << "Failed to seek OggOpus file:" << seekResult;
    }
    return getCurrentFrameIndex();
}

AudioSource::size_type AudioSourceOpus::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));

    size_type readCount = 0;
    while (readCount < numberOfFrames) {
        int readResult = op_read_float(m_pOggOpusFile,
            sampleBuffer + frames2samples(readCount),
            frames2samples(numberOfFrames - readCount), NULL);
        if (0 == readResult) {
            // EOF
            break; // done
        }
        if (0 < readResult) {
            readCount += readResult;
        } else {
            qWarning()
                << "Failed to read sample data from OggOpus file:"
                << readResult;
            break; // abort
        }
    }
    return readCount;
}

AudioSource::size_type AudioSourceOpus::readSampleFramesStereo(
    size_type numberOfFrames, sample_type* sampleBuffer, size_type sampleBufferSize) {
    DEBUG_ASSERT(isValidFrameIndex(getCurrentFrameIndex()));

    const size_type numberOfFramesTotal =
        math_min(numberOfFrames, samples2frames(sampleBufferSize));
    size_type numberOfFramesRead = 0;
    while (numberOfFramesTotal > numberOfFramesRead) {
        int readResult = op_read_float_stereo(m_pOggOpusFile,
            sampleBuffer + (numberOfFramesRead * 2),
            (numberOfFramesTotal - numberOfFramesRead) * 2);
        if (0 == readResult) {
            // EOF
            break; // done
        }
        if (0 < readResult) {
            numberOfFramesRead += readResult;
        } else {
            qWarning()
                << "Failed to read sample data from OggOpus file:"
                << readResult;
            break; // abort
        }
    }
    return numberOfFramesRead;
}

} // namespace Mixxx
