#include <QFile>

#include "sources/soundsourceoggvorbis.h"

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceOggVorbis");

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
        kLogger.warning()
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
        kLogger.warning()
            << "Unsupported format in"
            << getUrlString();
        return OpenResult::ABORTED;
    default:
        kLogger.warning()
            << "Failed to initialize decoder for"
            << getUrlString();
        return OpenResult::FAILED;
    }

    if (!ov_seekable(&m_vf)) {
        kLogger.warning()
                << "Stream in"
                << getUrlString()
                << "is not seekable";
        return OpenResult::ABORTED;
    }

    // lookup the ogg's channels and sample rate
    const vorbis_info* vi = ov_info(&m_vf, kCurrentBitstreamLink);
    if (!vi) {
        kLogger.warning()
                << "Failed to read stream info from"
                << getUrlString();
        return OpenResult::FAILED;
    }
    setChannelCount(vi->channels);
    setSamplingRate(vi->rate);
    if (0 < vi->bitrate_nominal) {
        initBitrate(vi->bitrate_nominal / 1000);
    } else {
        if ((0 < vi->bitrate_lower) && (vi->bitrate_lower == vi->bitrate_upper)) {
            initBitrate(vi->bitrate_lower / 1000);
        }
    }

    ogg_int64_t pcmTotal = ov_pcm_total(&m_vf, kEntireBitstreamLink);
    if (0 <= pcmTotal) {
        initFrameIndexRange(mixxx::IndexRange::forward(0, pcmTotal));
    } else {
        kLogger.warning()
                << "Failed to read read total length of"
                << getUrlString();
        return OpenResult::FAILED;
    }

    return OpenResult::SUCCEEDED;
}

void SoundSourceOggVorbis::close() {
    const int clearResult = ov_clear(&m_vf);
    if (0 != clearResult) {
        kLogger.warning() << "Failed to close file" << clearResult;
    }
    m_pFile.reset();
}

IndexRange SoundSourceOggVorbis::readOrSkipSampleFrames(
        IndexRange frameIndexRange,
        SampleBuffer::WritableSlice* pOutputBuffer) {
    auto readableFrames =
            adjustReadableFrameIndexRangeAndOutputBuffer(
                    frameIndexRange, pOutputBuffer);
    if (readableFrames.empty()) {
        return readableFrames;
    }

    if (m_curFrameIndex != readableFrames.start()) {
        const int seekResult = ov_pcm_seek(&m_vf, readableFrames.start());
        if (seekResult == 0) {
            m_curFrameIndex = readableFrames.start();
        } else {
            kLogger.warning() << "Failed to seek file:" << seekResult;
            const ogg_int64_t pcmOffset = ov_pcm_tell(&m_vf);
            if (0 <= pcmOffset) {
                m_curFrameIndex = pcmOffset;
            } else {
                // Reset to EOF
                m_curFrameIndex = frameIndexMax();
            }
            return IndexRange();
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == readableFrames.start());

    CSAMPLE* pSampleBuffer = pOutputBuffer ?
            pOutputBuffer->data() : nullptr;
    SINT numberOfFramesRemaining = readableFrames.length();
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
            if (pSampleBuffer) {
                if (channelCount().isMono()) {
                    for (long i = 0; i < readResult; ++i) {
                        *pSampleBuffer++ = pcmChannels[0][i];
                    }
                } else if (channelCount().isStereo()) {
                    for (long i = 0; i < readResult; ++i) {
                        *pSampleBuffer++ = pcmChannels[0][i];
                        *pSampleBuffer++ = pcmChannels[1][i];
                    }
                } else {
                    for (long i = 0; i < readResult; ++i) {
                        for (SINT j = 0; j < channelCount(); ++j) {
                            *pSampleBuffer++ = pcmChannels[j][i];
                        }
                    }
                }
            }
            numberOfFramesRemaining -= readResult;
        } else {
            kLogger.warning() << "Failed to read from file:" << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(readableFrames.length() >= numberOfFramesRemaining);
    return readableFrames.splitFront(readableFrames.length() - numberOfFramesRemaining);
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
