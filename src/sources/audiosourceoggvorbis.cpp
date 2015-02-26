#include "sources/audiosourceoggvorbis.h"

namespace Mixxx {

namespace {

// Parameter for ov_info()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_info.html
const int kCurrentBitstreamLink = -1; // retrieve ... for the current bitstream

// Parameter for ov_pcm_total()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_pcm_total.html
const int kEntireBitstreamLink  = -1; // retrieve ... for the entire physical bitstream

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
    const QByteArray qbaFilename(getLocalFileNameBytes());
    if (0 != ov_fopen(qbaFilename.constData(), &m_vf)) {
        qWarning() << "Failed to open OggVorbis file:" << getUrl();
        return ERR;
    }

    if (!ov_seekable(&m_vf)) {
        qWarning() << "OggVorbis file is not seekable:" << getUrl();
        return ERR;
    }

    // lookup the ogg's channels and sample rate
    const vorbis_info* vi = ov_info(&m_vf, kCurrentBitstreamLink);
    if (!vi) {
        qWarning() << "Failed to read OggVorbis file:" << getUrl();
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

    ogg_int64_t pcmTotal = ov_pcm_total(&m_vf, kEntireBitstreamLink);
    if (0 <= pcmTotal) {
        setFrameCount(pcmTotal);
    } else {
        qWarning() << "Failed to read total length of OggVorbis file:" << getUrl();
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
    if (0 == seekResult) {
        m_curFrameIndex = frameIndex;
    } else {
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
        size_type numberOfFrames, CSAMPLE* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer,
            frames2samples(numberOfFrames), false);
}

AudioSource::size_type AudioSourceOggVorbis::readSampleFramesStereo(
        size_type numberOfFrames, CSAMPLE* sampleBuffer,
        size_type sampleBufferSize) {
    return readSampleFrames(numberOfFrames, sampleBuffer, sampleBufferSize,
            true);
}

AudioSource::size_type AudioSourceOggVorbis::readSampleFrames(
        size_type numberOfFrames, CSAMPLE* sampleBuffer,
        size_type sampleBufferSize, bool readStereoSamples) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, readStereoSamples) <= sampleBufferSize);

    const size_type numberOfFramesTotal = math_min(numberOfFrames,
            size_type(getFrameIndexMax() - m_curFrameIndex));

    CSAMPLE* pSampleBuffer = sampleBuffer;
    size_type numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        float** pcmChannels;
        int currentSection;
        // Use 'long' here, because ov_read_float() returns this type.
        // This is an exception from the rule not to any types with
        // differing sizes on different platforms.
        // https://bugs.launchpad.net/mixxx/+bug/1094143
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
