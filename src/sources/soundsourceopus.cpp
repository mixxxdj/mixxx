#include "sources/soundsourceopus.h"

namespace mixxx {

namespace {

// Decoded output of opusfile has a fixed sample rate of 48 kHz
const SINT kSamplingRate = 48000;

// Parameter for op_channel_count()
// See also: https://mf4.xiph.org/jenkins/view/opus/job/opusfile-unix/ws/doc/html/group__stream__info.html
const int kCurrentStreamLink = -1; // get ... of the current (stream) link

// Parameter for op_pcm_total() and op_bitrate()
// See also: https://mf4.xiph.org/jenkins/view/opus/job/opusfile-unix/ws/doc/html/group__stream__info.html
const int kEntireStreamLink  = -1; // get ... of the whole/entire stream

class OggOpusFileOwner {
public:
    explicit OggOpusFileOwner(OggOpusFile* pFile) :
            m_pFile(pFile) {
    }
    ~OggOpusFileOwner() {
        op_free(m_pFile);
    }
    operator OggOpusFile*() const {
        return m_pFile;
    }
private:
    OggOpusFileOwner(const OggOpusFileOwner&); // disable copy constructor
    OggOpusFile* const m_pFile;
};

} // anonymous namespace

SoundSourceOpus::SoundSourceOpus(const QUrl& url)
        : SoundSource(url, "opus"),
          m_pOggOpusFile(nullptr),
          m_curFrameIndex(getMinFrameIndex()) {
}

SoundSourceOpus::~SoundSourceOpus() {
    close();
}

Result SoundSourceOpus::parseTrackMetadataAndCoverArt(
        TrackMetadata* pTrackMetadata,
        QImage* pCoverArt) const {
    if (OK == SoundSource::parseTrackMetadataAndCoverArt(
            pTrackMetadata, pCoverArt)) {
        // Done if the default implementation in the base class
        // supports Opus files.
        return OK;
    }

    // Beginning with version 1.9.0 TagLib supports the Opus format.
    // Until this becomes the minimum version required by Mixxx tags
    // in .opus files must also be parsed using opusfile. The following
    // code should removed as soon as it is no longer needed!
    //
    // NOTE(uklotzde): The following code has been found in SoundSourceOpus
    // and will not be improved. We are aware of its shortcomings like
    // the lack of proper error handling.

    // From opus/opusfile.h
    // On Windows, this string must be UTF-8 (to allow access to
    // files whose names cannot be represented in the current
    // MBCS code page).
    // All other systems use the native character encoding.
#ifdef _WIN32
    QByteArray qBAFilename = getLocalFileName().toUtf8();
#else
    QByteArray qBAFilename = getLocalFileName().toLocal8Bit();
#endif

    int error = 0;
    OggOpusFileOwner l_ptrOpusFile(
            op_open_file(qBAFilename.constData(), &error));

    int i = 0;
    const OpusTags *l_ptrOpusTags = op_tags(l_ptrOpusFile, -1);

    pTrackMetadata->setChannels(op_channel_count(l_ptrOpusFile, -1));
    pTrackMetadata->setSampleRate(kSamplingRate);
    pTrackMetadata->setBitrate(op_bitrate(l_ptrOpusFile, -1) / 1000);
    // Cast to double is required for duration with sub-second precision
    const double dTotalFrames = op_pcm_total(l_ptrOpusFile, -1);
    pTrackMetadata->setDuration(dTotalFrames / pTrackMetadata->getSampleRate());

    bool hasDate = false;
    for (i = 0; i < l_ptrOpusTags->comments; ++i) {
        QString l_SWholeTag = QString(l_ptrOpusTags->user_comments[i]);
        QString l_STag = l_SWholeTag.left(l_SWholeTag.indexOf("="));
        QString l_SPayload = l_SWholeTag.right((l_SWholeTag.length() - l_SWholeTag.indexOf("=")) - 1);

        if (!l_STag.compare("ARTIST")) {
            pTrackMetadata->setArtist(l_SPayload);
        } else if (!l_STag.compare("ALBUM")) {
            pTrackMetadata->setAlbum(l_SPayload);
        } else if (!l_STag.compare("BPM")) {
            pTrackMetadata->setBpm(Bpm(l_SPayload.toDouble()));
        } else if (!l_STag.compare("DATE")) {
            // Prefer "DATE" over "YEAR"
            pTrackMetadata->setYear(l_SPayload.trimmed());
            // Avoid to overwrite "DATE" with "YEAR"
            hasDate |= !pTrackMetadata->getYear().isEmpty();
        } else if (!hasDate && !l_STag.compare("YEAR")) {
            pTrackMetadata->setYear(l_SPayload.trimmed());
        } else if (!l_STag.compare("GENRE")) {
            pTrackMetadata->setGenre(l_SPayload);
        } else if (!l_STag.compare("TRACKNUMBER")) {
            pTrackMetadata->setTrackNumber(l_SPayload);
        } else if (!l_STag.compare("COMPOSER")) {
            pTrackMetadata->setComposer(l_SPayload);
        } else if (!l_STag.compare("ALBUMARTIST")) {
            pTrackMetadata->setAlbumArtist(l_SPayload);
        } else if (!l_STag.compare("TITLE")) {
            pTrackMetadata->setTitle(l_SPayload);
        } else if (!l_STag.compare("REPLAYGAIN_TRACK_GAIN")) {
            bool trackGainRatioValid = false;
            double trackGainRatio = ReplayGain::ratioFromString(l_SPayload, &trackGainRatioValid);
            if (trackGainRatioValid) {
                ReplayGain trackGain(pTrackMetadata->getReplayGain());
                trackGain.setRatio(trackGainRatio);
                pTrackMetadata->setReplayGain(trackGain);
            }
        }

        // This is left fot debug reasons!!
        //qDebug() << "Comment" << i << l_ptrOpusTags->comment_lengths[i] <<
        //" (" << l_ptrOpusTags->user_comments[i] << ")" << l_STag << "*" << l_SPayload;
    }

    return OK;
}

SoundSource::OpenResult SoundSourceOpus::tryOpen(const AudioSourceConfig& /*audioSrcCfg*/) {
    // From opus/opusfile.h
    // On Windows, this string must be UTF-8 (to allow access to
    // files whose names cannot be represented in the current
    // MBCS code page).
    // All other systems use the native character encoding.
#ifdef _WIN32
    QByteArray qBAFilename = getLocalFileName().toUtf8();
#else
    QByteArray qBAFilename = getLocalFileName().toLocal8Bit();
#endif

    int errorCode = 0;

    DEBUG_ASSERT(!m_pOggOpusFile);
    m_pOggOpusFile = op_open_file(qBAFilename.constData(), &errorCode);
    if (!m_pOggOpusFile) {
        qWarning() << "Failed to open OggOpus file:" << getUrlString()
                << "errorCode" << errorCode;
        return OpenResult::FAILED;
    }

    if (!op_seekable(m_pOggOpusFile)) {
        qWarning() << "SoundSourceOpus:"
                << "Stream in"
                << getUrlString()
                << "is not seekable";
        return OpenResult::UNSUPPORTED_FORMAT;
    }

    const int channelCount = op_channel_count(m_pOggOpusFile, kCurrentStreamLink);
    if (0 < channelCount) {
        setChannelCount(channelCount);
    } else {
        qWarning() << "Failed to read channel configuration of OggOpus file:" << getUrlString();
        return OpenResult::FAILED;
    }

    const ogg_int64_t pcmTotal = op_pcm_total(m_pOggOpusFile, kEntireStreamLink);
    if (0 <= pcmTotal) {
        setFrameCount(pcmTotal);
    } else {
        qWarning() << "Failed to read total length of OggOpus file:" << getUrlString();
        return OpenResult::FAILED;
    }

    const opus_int32 bitrate = op_bitrate(m_pOggOpusFile, kEntireStreamLink);
    if (0 < bitrate) {
        setBitrate(bitrate / 1000);
    } else {
        qWarning() << "Failed to determine bitrate of OggOpus file:" << getUrlString();
        return OpenResult::FAILED;
    }

    setSamplingRate(kSamplingRate);

    m_curFrameIndex = getMinFrameIndex();

    return OpenResult::SUCCEEDED;
}

void SoundSourceOpus::close() {
    if (m_pOggOpusFile) {
        op_free(m_pOggOpusFile);
        m_pOggOpusFile = nullptr;
    }
}

SINT SoundSourceOpus::seekSampleFrame(SINT frameIndex) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(isValidFrameIndex(frameIndex));

    int seekResult = op_pcm_seek(m_pOggOpusFile, frameIndex);
    if (0 == seekResult) {
        m_curFrameIndex = frameIndex;
    } else {
        qWarning() << "Failed to seek OggOpus file:" << seekResult;
        const ogg_int64_t pcmOffset = op_pcm_tell(m_pOggOpusFile);
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

SINT SoundSourceOpus::readSampleFrames(
        SINT numberOfFrames, CSAMPLE* sampleBuffer) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));

    const SINT numberOfFramesTotal = math_min(
            numberOfFrames, getMaxFrameIndex() - m_curFrameIndex);

    CSAMPLE* pSampleBuffer = sampleBuffer;
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        int readResult = op_read_float(m_pOggOpusFile,
                pSampleBuffer,
                frames2samples(numberOfFramesRemaining), nullptr);
        if (0 < readResult) {
            m_curFrameIndex += readResult;
            pSampleBuffer += frames2samples(readResult);
            numberOfFramesRemaining -= readResult;
        } else {
            qWarning() << "Failed to read sample data from OggOpus file:"
                    << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

SINT SoundSourceOpus::readSampleFramesStereo(
        SINT numberOfFrames, CSAMPLE* sampleBuffer,
        SINT sampleBufferSize) {
    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(getSampleBufferSize(numberOfFrames, true) <= sampleBufferSize);

    const SINT numberOfFramesTotal = math_min(
            numberOfFrames, getMaxFrameIndex() - m_curFrameIndex);

    CSAMPLE* pSampleBuffer = sampleBuffer;
    SINT numberOfFramesRemaining = numberOfFramesTotal;
    while (0 < numberOfFramesRemaining) {
        int readResult = op_read_float_stereo(m_pOggOpusFile,
                pSampleBuffer,
                numberOfFramesRemaining * 2); // stereo
        if (0 < readResult) {
            m_curFrameIndex += readResult;
            pSampleBuffer += readResult * 2; // stereo
            numberOfFramesRemaining -= readResult;
        } else {
            qWarning() << "Failed to read sample data from OggOpus file:"
                    << readResult;
            break; // abort
        }
    }

    DEBUG_ASSERT(isValidFrameIndex(m_curFrameIndex));
    DEBUG_ASSERT(numberOfFramesTotal >= numberOfFramesRemaining);
    return numberOfFramesTotal - numberOfFramesRemaining;
}

QString SoundSourceProviderOpus::getName() const {
    return "Xiph.org libopusfile";
}

QStringList SoundSourceProviderOpus::getSupportedFileExtensions() const {
    QStringList supportedFileExtensions;
    supportedFileExtensions.append("opus");
    return supportedFileExtensions;
}

} // namespace mixxx
