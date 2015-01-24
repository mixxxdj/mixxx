#include "sources/audiosourceoggvorbis.h"

namespace Mixxx {

namespace {

const int kLogicalBitstreamIndex = -1; // whole stream/file

} // anonymous namespace

AudioSourceOggVorbis::AudioSourceOggVorbis(QUrl url)
        : AudioSource(url),
          m_curFrameIndex(0) {
    memset(&m_vf, 0, sizeof(m_vf));
}

AudioSourceOggVorbis::~AudioSourceOggVorbis() {
    preDestroy();
}

AudioSourcePointer AudioSourceOggVorbis::create(QUrl url) {
    return onCreate(new AudioSourceOggVorbis(url));
}

Result AudioSourceOggVorbis::postConstruct() {
    const QString fileName(getUrl().toLocalFile());
    const QByteArray qbaFilename(fileName.toLocal8Bit());
    if (0 != ov_fopen(qbaFilename.constData(), &m_vf)) {
        qWarning() << "Failed to open OggVorbis file:" << fileName;
        return ERR;
    }

    if (!ov_seekable(&m_vf)) {
        qWarning() << "OggVorbis file is not seekable:" << fileName;
        return ERR;
    }

    // lookup the ogg's channels and sample rate
    const vorbis_info* vi = ov_info(&m_vf, kLogicalBitstreamIndex);
    if (!vi) {
        qWarning() << "Failed to read OggVorbis file:" << fileName;
        return ERR;
    }
    setChannelCount(vi->channels);
    setFrameRate(vi->rate);
    if (0 < vi->bitrate_nominal) {
        setBitrate(vi->bitrate_nominal / 1000);
    } else {
        if ((0 < vi->bitrate_lower) && (vi->bitrate_lower == vi->bitrate_upper)) {
            setBitrate(vi->bitrate_lower / 1000);
        }
    }

    ogg_int64_t pcmTotal = ov_pcm_total(&m_vf, kLogicalBitstreamIndex);
    if (0 <= pcmTotal) {
        setFrameCount(pcmTotal);
    } else {
        qWarning() << "Failed to read total length of OggVorbis file:" << fileName;
        return ERR;
    }

    return OK;
}

void AudioSourceOggVorbis::preDestroy() {
    const int clearResult = ov_clear(&m_vf);
    if (0 != clearResult) {
        qWarning() << "Failed to close OggVorbis file" << clearResult;
    }
}

AudioSource::diff_type AudioSourceOggVorbis::seekSampleFrame(
        diff_type frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    const int seekResult = ov_pcm_seek(&m_vf, frameIndex);
    if (0 != seekResult) {
        qWarning() << "Failed to seek OggVorbis file:" << seekResult;
        const ogg_int64_t pcmOffset = ov_pcm_tell(&m_vf);
        if (0 <= pcmOffset) {
            m_curFrameIndex = pcmOffset;
        } else {
            // Reset to EOF
            m_curFrameIndex = getFrameIndexMax();
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    return m_curFrameIndex;
}

AudioSource::size_type AudioSourceOggVorbis::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer,
            frames2samples(numberOfFrames), false);
}

AudioSource::size_type AudioSourceOggVorbis::readSampleFramesStereo(
        size_type numberOfFrames, sample_type* sampleBuffer,
        size_type sampleBufferSize) {
    return readSampleFrames(numberOfFrames, sampleBuffer, sampleBufferSize,
            true);
}

AudioSource::size_type AudioSourceOggVorbis::readSampleFrames(
        size_type numberOfFrames, sample_type* sampleBuffer,
        size_type sampleBufferSize, bool readStereoSamples) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, readStereoSamples) >= sampleBufferSize);

    const size_type numberOfFramesTotal = math_min(numberOfFrames,
            size_type(getFrameIndexMax() - m_curFrameIndex));

    sample_type* pSampleBuffer = sampleBuffer;
    size_type numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        float** pcmChannels;
        int currentSection;
        const long readResult = ov_read_float(&m_vf, &pcmChannels,
                numberOfFramesRemaining, &currentSection);
        if (0 < readResult) {
            m_curFrameIndex += readResult;
            if (isChannelCountMono()) {
                if (readStereoSamples) {
                    for (long i = 0; i < readResult; ++i) {
                        *pSampleBuffer++ = pcmChannels[0][i];
                        *pSampleBuffer++ = pcmChannels[0][i];
                    }
                } else {
                    for (long i = 0; i < readResult; ++i) {
                        *pSampleBuffer++ = pcmChannels[0][i];
                    }
                }
            } else if (isChannelCountStereo() || readStereoSamples) {
                for (long i = 0; i < readResult; ++i) {
                    *pSampleBuffer++ = pcmChannels[0][i];
                    *pSampleBuffer++ = pcmChannels[1][i];
                }
            } else {
                for (long i = 0; i < readResult; ++i) {
                    for (size_type j = 0; j < getChannelCount(); ++j) {
                        *pSampleBuffer++ = pcmChannels[j][i];
                    }
                }
            }
            numberOfFramesRemaining -= readResult;
        } else {
            qWarning() << "Failed to read from OggVorbis file:" << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

} // namespace Mixxx
