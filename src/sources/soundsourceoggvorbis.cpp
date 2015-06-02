#include "sources/soundsourceoggvorbis.h"

namespace Mixxx {

namespace {

// Parameter for ov_info()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_info.html
const int kCurrentBitstreamLink = -1; // retrieve ... for the current bitstream

// Parameter for ov_pcm_total()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_pcm_total.html
const int kEntireBitstreamLink  = -1; // retrieve ... for the entire physical bitstream

} // anonymous namespace

<<<<<<< HEAD
QList<QString> SoundSourceOggVorbis::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("ogg");
    return list;
}

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename)
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
        : Mixxx::SoundSource(qFilename),
          channels(0),
          filelength(0),
          current_section(0) {
    setType("ogg");
    vf.datasource = NULL;
    vf.seekable = 0;
    vf.offset = 0;
    vf.end = 0;
    //vf.oy
    vf.links = 0;
    vf.offsets = NULL;
    vf.dataoffsets = NULL;
    vf.serialnos = NULL;
    vf.pcmlengths = NULL;
    vf.vi = NULL;
    vf.vc = NULL;
    vf.pcm_offset = 0;
    vf.ready_state = 0;
    vf.current_serialno = 0;
    vf.current_link = 0;
    vf.bittrack = 0;
    vf.samptrack = 0;
    //vf.os
    //vf.vd
    //vf.vb
    //vf.callbacks
=======
        : Super(qFilename, "ogg") {
<<<<<<< HEAD
    memset(&m_vf, 0, sizeof(m_vf));
>>>>>>> New SoundSource/AudioSource API
}

SoundSourceOggVorbis::~SoundSourceOggVorbis() {
    if (0 != ov_clear(&m_vf)) {
        qWarning() << "Failed to close OggVorbis file:" << getFilename();
    }
}

Result SoundSourceOggVorbis::open() {

    if (0 != ov_fopen(getFilename().toLocal8Bit().constData(), &m_vf)) {
        qWarning() << "Failed to open OggVorbis file:" << getFilename();
=======
=======
>>>>>>> Add and register a SoundSourceProvider for each SoundSource
SoundSourceOggVorbis::SoundSourceOggVorbis(QUrl url)
        : SoundSource(url, "ogg"),
          m_curFrameIndex(0) {
    memset(&m_vf, 0, sizeof(m_vf));
}

SoundSourceOggVorbis::~SoundSourceOggVorbis() {
    close();
}

Result SoundSourceOggVorbis::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    const QByteArray qbaFilename(getLocalFileNameBytes());
    if (0 != ov_fopen(qbaFilename.constData(), &m_vf)) {
<<<<<<< HEAD
        qWarning() << "Failed to open OggVorbis file:" << getUrl();
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
=======
        qWarning() << "Failed to open OggVorbis file:" << getUrlString();
>>>>>>> Fix release build
        return ERR;
    }

    if (!ov_seekable(&m_vf)) {
<<<<<<< HEAD
<<<<<<< HEAD
        qWarning() << "OggVorbis file is not seekable:" << getFilename();
        close();
        return ERR;
    }

    // lookup the ogg's channels and samplerate
    const vorbis_info* vi = ov_info(&m_vf, -1);
    if (!vi) {
        qWarning() << "Failed to read OggVorbis file:" << getFilename();
        close();
=======
        qWarning() << "OggVorbis file is not seekable:" << getUrl();
=======
        qWarning() << "OggVorbis file is not seekable:" << getUrlString();
>>>>>>> Fix release build
        return ERR;
    }

    // lookup the ogg's channels and sample rate
    const vorbis_info* vi = ov_info(&m_vf, kCurrentBitstreamLink);
    if (!vi) {
<<<<<<< HEAD
        qWarning() << "Failed to read OggVorbis file:" << getUrl();
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
=======
        qWarning() << "Failed to read OggVorbis file:" << getUrlString();
>>>>>>> Fix release build
        return ERR;
    }
    setChannelCount(vi->channels);
    setFrameRate(vi->rate);
<<<<<<< HEAD

    ogg_int64_t frameCount = ov_pcm_total(&m_vf, -1);
    if (0 <= frameCount) {
        setFrameCount(frameCount);
    } else {
        qWarning() << "Failed to read OggVorbis file:" << getFilename();
        close();
=======
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
<<<<<<< HEAD
        qWarning() << "Failed to read total length of OggVorbis file:" << getUrl();
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
=======
        qWarning() << "Failed to read total length of OggVorbis file:" << getUrlString();
>>>>>>> Fix release build
        return ERR;
    }

    return OK;
<<<<<<< HEAD
}

void SoundSourceOggVorbis::close() {
    if (0 != ov_clear(&m_vf)) {
        qWarning() << "Failed to close OggVorbis file:" << getFilename();
    }
    Super::reset();
}

Mixxx::AudioSource::diff_type SoundSourceOggVorbis::seekFrame(
        diff_type frameIndex) {
    int seekResult = ov_pcm_seek(&m_vf, frameIndex);
    if (0 != seekResult) {
        qWarning() << "Failed to seek OggVorbis file:" << getFilename();
    }
    return ov_pcm_tell(&m_vf);
}

Mixxx::AudioSource::size_type SoundSourceOggVorbis::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, false);
}

Mixxx::AudioSource::size_type SoundSourceOggVorbis::readStereoFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    return readFrameSamplesInterleaved(frameCount, sampleBuffer, true);
}

Mixxx::AudioSource::size_type SoundSourceOggVorbis::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer,
        bool readStereoSamples) {
    size_type readCount = 0;
    sample_type* nextSample = sampleBuffer;
    while (readCount < frameCount) {
        float** pcmChannels;
        int currentSection;
        long readResult = ov_read_float(&m_vf, &pcmChannels,
                frameCount - readCount, &currentSection);
        if (0 == readResult) {
            break; // done
        }
        if (0 < readResult) {
            if (isChannelCountMono()) {
                if (readStereoSamples) {
                    for (long i = 0; i < readResult; ++i) {
                        *nextSample++ = pcmChannels[0][i];
                        *nextSample++ = pcmChannels[0][i];
                    }
                } else {
                    for (long i = 0; i < readResult; ++i) {
                        *nextSample++ = pcmChannels[0][i];
                    }
                }
            } else if (isChannelCountStereo() || readStereoSamples) {
                for (long i = 0; i < readResult; ++i) {
                    *nextSample++ = pcmChannels[0][i];
                    *nextSample++ = pcmChannels[1][i];
                }
            } else {
                for (long i = 0; i < readResult; ++i) {
                    for (size_type j = 0; j < getChannelCount(); ++j) {
                        *nextSample++ = pcmChannels[j][i];
                    }
                }
            }
            readCount += readResult;
        } else {
            qWarning() << "Failed to read sample data from OggVorbis file:"
                    << getFilename();
            break; // abort
        }
    }
    return readCount;
=======
>>>>>>> Split AudioSource from SoundSource
=======
        : SoundSource(qFilename, "ogg") {
>>>>>>> Delete typedef Super (review comments)
=======
    : SoundSource(qFilename, "ogg") {
>>>>>>> Fix coding style issues (indentation)
=======
SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename) :
        SoundSource(qFilename, "ogg") {
>>>>>>> Reformat source code (roughly K&R + spaces)
=======
SoundSourceOggVorbis::SoundSourceOggVorbis(QUrl url) :
        SoundSource(url, "ogg") {
>>>>>>> Create SoundSource from URL
=======
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
}

void SoundSourceOggVorbis::close() {
    const int clearResult = ov_clear(&m_vf);
    if (0 != clearResult) {
        qWarning() << "Failed to close OggVorbis file" << clearResult;
    }
}

SINT SoundSourceOggVorbis::seekSampleFrame(
        SINT frameIndex) {
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
            m_curFrameIndex = getMaxFrameIndex();
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    return m_curFrameIndex;
}

SINT SoundSourceOggVorbis::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    return readSampleFrames(numberOfFrames, sampleBuffer,
            frames2samples(numberOfFrames), false);
}

SINT SoundSourceOggVorbis::readSampleFramesStereo(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize) {
    return readSampleFrames(numberOfFrames, sampleBuffer, sampleBufferSize,
            true);
}

SINT SoundSourceOggVorbis::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize, bool readStereoSamples) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, readStereoSamples) <= sampleBufferSize);

    const SINT numberOfFramesTotal = math_min(
            numberOfFrames, getMaxFrameIndex() - m_curFrameIndex);

    CSAMPLE* pSampleBuffer = sampleBuffer;
    SINT numberOfFramesRemaining = numberOfFramesTotal;
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
            if (kChannelCountMono == getChannelCount()) {
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
            } else if (readStereoSamples || (kChannelCountStereo == getChannelCount())) {
                for (long i = 0; i < readResult; ++i) {
                    *pSampleBuffer++ = pcmChannels[0][i];
                    *pSampleBuffer++ = pcmChannels[1][i];
                }
            } else {
                for (long i = 0; i < readResult; ++i) {
                    for (SINT j = 0; j < getChannelCount(); ++j) {
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

QString SoundSourceProviderOggVorbis::getName() const {
    return "Xiph.org OggVorbis";
}

QStringList SoundSourceProviderOggVorbis::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("ogg");
    return supportedFileExtensions;
}

} // namespace Mixxx
