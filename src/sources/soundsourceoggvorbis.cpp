#include "sources/soundsourceoggvorbis.h"

#include <QFile>

namespace mixxx {

namespace {

// Parameter for ov_info()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_info.html
const int kCurrentBitstreamLink = -1; // retrieve ... for the current bitstream

// Parameter for ov_pcm_total()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_pcm_total.html
const int kEntireBitstreamLink  = -1; // retrieve ... for the entire physical bitstream

} // anonymous namespace

//static
ov_callbacks SoundSourceOggVorbis::s_callbacks = {
    SoundSourceOggVorbis::ReadCallback,
    SoundSourceOggVorbis::SeekCallback,
    SoundSourceOggVorbis::CloseCallback,
    SoundSourceOggVorbis::TellCallback
};

SoundSourceOggVorbis::SoundSourceOggVorbis(const QUrl& url)
        : SoundSource(url, "ogg"),
          m_curFrameIndex(0) {
    memset(&m_vf, 0, sizeof(m_vf));
}

SoundSourceOggVorbis::~SoundSourceOggVorbis() {
    close();
}

SoundSource::OpenResult SoundSourceOggVorbis::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    m_pFile = std::make_unique<QFile>(getLocalFileName());
    if(!m_pFile->open(QFile::ReadOnly)) {
        qWarning() << "SoundSourceOggVorbis:"
                << "Failed to open file for"
                << getUrlString();
        return OpenResult::FAILED;
    }

    const int initDecoderResult = ov_open_callbacks(m_pFile.get(), &m_vf, nullptr, 0, s_callbacks);
    switch (initDecoderResult) {
    case 0:
        // success -> continue
        break;
    case OV_ENOTVORBIS:
    case OV_EVERSION:
        qWarning() << "SoundSourceOggVorbis:"
            << "Unsupported format in"
            << getUrlString();
        return OpenResult::ABORTED;
    default:
        qWarning() << "SoundSourceOggVorbis:"
            << "Failed to initialize decoder for"
            << getUrlString();
        return OpenResult::FAILED;
    }

    if (!ov_seekable(&m_vf)) {
        qWarning() << "SoundSourceOggVorbis:"
                << "Stream in"
                << getUrlString()
                << "is not seekable";
        return OpenResult::ABORTED;
    }

    // lookup the ogg's channels and sample rate
    const vorbis_info* vi = ov_info(&m_vf, kCurrentBitstreamLink);
    if (!vi) {
        qWarning() << "SoundSourceOggVorbis:"
                << "Failed to read stream info from"
                << getUrlString();
        return OpenResult::FAILED;
    }
    setChannelCount(vi->channels);
    setSamplingRate(vi->rate);
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
        qWarning() << "SoundSourceOggVorbis:"
                << "Failed to read read total length of"
                << getUrlString();
        return OpenResult::FAILED;
    }

    return OpenResult::SUCCEEDED;
}

void SoundSourceOggVorbis::close() {
    const int clearResult = ov_clear(&m_vf);
    if (0 != clearResult) {
        qWarning() << "Failed to close OggVorbis file" << clearResult;
    }
    m_pFile.reset();
}

SINT SoundSourceOggVorbis::seekSampleFrame(
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
            if (pSampleBuffer != nullptr) {
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


//static
size_t SoundSourceOggVorbis::ReadCallback(void *ptr, size_t size, size_t nmemb,
       void *datasource) {
   if (!size || !nmemb) {
       return 0;
   }
   QFile* pFile = static_cast<QFile*>(datasource);
   if (!pFile) {
       return 0;
   }

   nmemb = math_min<size_t>((pFile->size() - pFile->pos()) / size, nmemb);
   pFile->read((char*)ptr, nmemb * size);
   return nmemb;
}

//static
int SoundSourceOggVorbis::SeekCallback(void *datasource, ogg_int64_t offset,
       int whence) {
   QFile* pFile = static_cast<QFile*>(datasource);
   if (!pFile) {
       return 0;
   }

   switch(whence) {
   case SEEK_SET:
       return pFile->seek(offset) ? 0 : -1;
   case SEEK_CUR:
       return pFile->seek(pFile->pos() + offset) ? 0 : -1;
   case SEEK_END:
       return pFile->seek(pFile->size() + offset) ? 0 : -1;
   default:
       return -1;
   }
}

//static
int SoundSourceOggVorbis::CloseCallback(void* datasource) {
   QFile* pFile = static_cast<QFile*>(datasource);
   if (!pFile) {
       return 0;
   }
   pFile->close();
   return 0;
}

//static
long SoundSourceOggVorbis::TellCallback(void* datasource) {
   QFile* pFile = static_cast<QFile*>(datasource);
   if (!pFile) {
       return 0;
   }
   return pFile->pos();
}

QString SoundSourceProviderOggVorbis::getName() const {
    return "Xiph.org OggVorbis";
}

QStringList SoundSourceProviderOggVorbis::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("ogg");
    return supportedFileExtensions;
}

} // namespace mixxx
