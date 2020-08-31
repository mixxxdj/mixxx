#include "sources/soundsourceoggvorbis.h"

#include <QFile>

#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("SoundSourceOggVorbis");

// Parameter for ov_info()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_info.html
constexpr int kCurrentBitstreamLink = -1; // retrieve ... for the current bitstream

// Parameter for ov_pcm_total()
// See also: https://xiph.org/vorbis/doc/vorbisfile/ov_pcm_total.html
constexpr int kEntireBitstreamLink = -1; // retrieve ... for the entire physical bitstream

} // anonymous namespace

//static
ov_callbacks SoundSourceOggVorbis::s_callbacks = {
        SoundSourceOggVorbis::ReadCallback,
        SoundSourceOggVorbis::SeekCallback,
        SoundSourceOggVorbis::CloseCallback,
        SoundSourceOggVorbis::TellCallback};

//static
const QString SoundSourceProviderOggVorbis::kDisplayName = QStringLiteral("Xiph.org OggVorbis");

//static
const QStringList SoundSourceProviderOggVorbis::kSupportedFileExtensions = {
        QStringLiteral("ogg"),
};

SoundSourceProviderPriority SoundSourceProviderOggVorbis::getPriorityHint(
        const QString& supportedFileExtension) const {
    Q_UNUSED(supportedFileExtension)
    // This reference decoder is supposed to produce more accurate
    // and reliable results than any other DEFAULT provider.
    return SoundSourceProviderPriority::Higher;
}

SoundSourceOggVorbis::SoundSourceOggVorbis(const QUrl& url)
        : SoundSource(url),
          m_curFrameIndex(0) {
    memset(&m_vf, 0, sizeof(m_vf));
}

SoundSourceOggVorbis::~SoundSourceOggVorbis() {
    close();
}

SoundSource::OpenResult SoundSourceOggVorbis::tryOpen(
        OpenMode /*mode*/,
        const OpenParams& /*config*/) {
    m_pFile = std::make_unique<QFile>(getLocalFileName());
    if (!m_pFile->open(QFile::ReadOnly)) {
        kLogger.warning()
                << "Failed to open file for"
                << getUrlString();
        return OpenResult::Failed;
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
        return OpenResult::Aborted;
    default:
        kLogger.warning()
                << "Failed to initialize decoder for"
                << getUrlString();
        return OpenResult::Failed;
    }

    if (!ov_seekable(&m_vf)) {
        kLogger.warning()
                << "Stream in"
                << getUrlString()
                << "is not seekable";
        return OpenResult::Aborted;
    }

    // lookup the ogg's channels and sample rate
    const vorbis_info* vi = ov_info(&m_vf, kCurrentBitstreamLink);
    if (!vi) {
        kLogger.warning()
                << "Failed to read stream info from"
                << getUrlString();
        return OpenResult::Failed;
    }
    initChannelCountOnce(vi->channels);
    initSampleRateOnce(vi->rate);
    if (0 < vi->bitrate_nominal) {
        initBitrateOnce(vi->bitrate_nominal / 1000);
    } else {
        if ((0 < vi->bitrate_lower) && (vi->bitrate_lower == vi->bitrate_upper)) {
            initBitrateOnce(vi->bitrate_lower / 1000);
        }
    }

    ogg_int64_t pcmTotal = ov_pcm_total(&m_vf, kEntireBitstreamLink);
    if (0 <= pcmTotal) {
        initFrameIndexRangeOnce(IndexRange::forward(0, pcmTotal));
    } else {
        kLogger.warning()
                << "Failed to read read total length of"
                << getUrlString();
        return OpenResult::Failed;
    }

    return OpenResult::Succeeded;
}

void SoundSourceOggVorbis::close() {
    const int clearResult = ov_clear(&m_vf);
    if (0 != clearResult) {
        kLogger.warning() << "Failed to close file" << clearResult;
    }
    m_pFile.reset();
}

ReadableSampleFrames SoundSourceOggVorbis::readSampleFramesClamped(
        WritableSampleFrames writableSampleFrames) {
    const SINT firstFrameIndex = writableSampleFrames.frameIndexRange().start();

    if (m_curFrameIndex != firstFrameIndex) {
        const int seekResult = ov_pcm_seek(&m_vf, firstFrameIndex);
        if (seekResult == 0) {
            m_curFrameIndex = firstFrameIndex;
        } else {
            kLogger.warning() << "Failed to seek file:" << seekResult;
            const ogg_int64_t pcmOffset = ov_pcm_tell(&m_vf);
            if (0 <= pcmOffset) {
                m_curFrameIndex = pcmOffset;
            } else {
                // Reset to EOF
                m_curFrameIndex = frameIndexMax();
            }
            // Abort
            return ReadableSampleFrames(
                    IndexRange::between(
                            m_curFrameIndex,
                            m_curFrameIndex));
        }
    }
    DEBUG_ASSERT(m_curFrameIndex == firstFrameIndex);

    const SINT numberOfFramesTotal = writableSampleFrames.frameLength();

    CSAMPLE* pSampleBuffer = writableSampleFrames.writableData();
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        float** pcmChannels;
        int currentSection;
        // Use 'long' here, because ov_read_float() returns this type.
        // This is an exception from the rule not to any types with
        // differing sizes on different platforms.
        // https://bugs.launchpad.net/mixxx/+bug/1094143
        const long readResult = ov_read_float(&m_vf, &pcmChannels, numberOfFramesRemaining, &currentSection);
        if (0 < readResult) {
            m_curFrameIndex += readResult;
            if (pSampleBuffer) {
                switch (getSignalInfo().getChannelCount()) {
                case 1:
                    for (long i = 0; i < readResult; ++i) {
                        *pSampleBuffer++ = pcmChannels[0][i];
                    }
                    break;
                case 2:
                    for (long i = 0; i < readResult; ++i) {
                        *pSampleBuffer++ = pcmChannels[0][i];
                        *pSampleBuffer++ = pcmChannels[1][i];
                    }
                    break;
                default:
                    for (long i = 0; i < readResult; ++i) {
                        for (SINT j = 0; j < getSignalInfo().getChannelCount(); ++j) {
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
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    const SINT numberOfFrames = numberOfFramesTotal - numberOfFramesRemaining;
    return ReadableSampleFrames(
            IndexRange::forward(firstFrameIndex, numberOfFrames),
            SampleBuffer::ReadableSlice(
                    writableSampleFrames.writableData(),
                    std::min(
                            writableSampleFrames.writableLength(),
                            getSignalInfo().frames2samples(numberOfFrames))));
}

//static
size_t SoundSourceOggVorbis::ReadCallback(void* ptr, size_t size, size_t nmemb, void* datasource) {
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
int SoundSourceOggVorbis::SeekCallback(void* datasource, ogg_int64_t offset, int whence) {
    QFile* pFile = static_cast<QFile*>(datasource);
    if (!pFile) {
        return 0;
    }

    switch (whence) {
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

} // namespace mixxx
