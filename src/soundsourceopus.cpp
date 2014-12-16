#include "soundsourceopus.h"

#include "trackmetadatataglib.h"

// Include this if taglib if new enough (version 1.9.1 have opusfile)
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
#include <taglib/opusfile.h>
#endif

namespace {
// All Opus audio is encoded at 48 kHz
Mixxx::AudioSource::size_type kOpusSampleRate = 48000;
}

QList<QString> SoundSourceOpus::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("opus");
    return list;
}

SoundSourceOpus::SoundSourceOpus(QString qFilename)
        : Super(qFilename, "opus"), m_pOggOpusFile(NULL) {
}

SoundSourceOpus::~SoundSourceOpus() {
    close();
    if (m_pOggOpusFile) {
        op_free(m_pOggOpusFile);
    }
}

Result SoundSourceOpus::open() {
    const QByteArray qBAFilename(getFilename().toLocal8Bit());

    int errorCode = 0;
    m_pOggOpusFile = op_open_file(qBAFilename.constData(), &errorCode);
    if (!m_pOggOpusFile) {
        qDebug() << "Failed to open OggOpus file:" << getFilename()
                << "errorCode" << errorCode;
        return ERR;
    }

    if (!op_seekable(m_pOggOpusFile)) {
        qWarning() << "OggOpus file is not seekable:" << getFilename();
        close();
        return ERR;
    }

    setChannelCount(op_channel_count(m_pOggOpusFile, -1));
    setFrameRate(kOpusSampleRate);

    ogg_int64_t frameCount = op_pcm_total(m_pOggOpusFile, -1);
    if (0 <= frameCount) {
        setFrameCount(frameCount);
    } else {
        qWarning() << "Failed to read OggOpus file:" << getFilename();
        close();
        return ERR;
    }

    return OK;
}

void SoundSourceOpus::close() {
    if (m_pOggOpusFile) {
        op_free(m_pOggOpusFile);
        m_pOggOpusFile = NULL;
    }
    Super::reset();
}

Mixxx::AudioSource::diff_type SoundSourceOpus::seekFrame(diff_type frameIndex) {
    int seekResult = op_pcm_seek(m_pOggOpusFile, frameIndex);
    if (0 != seekResult) {
        qWarning() << "Failed to seek OggVorbis file:" << getFilename();
    }
    return op_pcm_tell(m_pOggOpusFile);
}

Mixxx::AudioSource::size_type SoundSourceOpus::readFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    size_type readCount = 0;
    while (readCount < frameCount) {
        int readResult = op_read_float(m_pOggOpusFile,
                sampleBuffer + frames2samples(readCount),
                frames2samples(frameCount - readCount), NULL);
        if (0 == readResult) {
            break; // done
        }
        if (0 < readResult) {
            readCount += readResult;
        } else {
            qWarning() << "Failed to read sample data from OggOpus file:"
                    << getFilename();
            break; // abort
        }
    }
    return readCount;
}

Mixxx::AudioSource::size_type SoundSourceOpus::readStereoFrameSamplesInterleaved(
        size_type frameCount, sample_type* sampleBuffer) {
    size_type readCount = 0;
    while (readCount < frameCount) {
        int readResult = op_read_float_stereo(m_pOggOpusFile,
                sampleBuffer + (readCount * 2),
                (frameCount - readCount) * 2);
        if (0 == readResult) {
            break; // done
        }
        if (0 < readResult) {
            readCount += readResult;
        } else {
            qWarning() << "Failed to read sample data from OggOpus file:"
                    << getFilename();
            break; // abort
        }
    }
    return readCount;
}

namespace
{
    class OggOpusFileOwner {
    public:
        explicit OggOpusFileOwner(OggOpusFile* pFile): m_pFile(pFile) {
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
}

/*
 Parse the the file to get metadata
 */
Result SoundSourceOpus::parseMetadata(Mixxx::TrackMetadata* pMetadata) {
    int error = 0;

    QByteArray qBAFilename = getFilename().toLocal8Bit();

    OggOpusFileOwner l_ptrOpusFile(op_open_file(qBAFilename.constData(), &error));

    pMetadata->setChannels(op_channel_count(l_ptrOpusFile, -1));
    pMetadata->setSampleRate(kOpusSampleRate);
    pMetadata->setBitrate(op_bitrate(l_ptrOpusFile, -1) / 1000);
    pMetadata->setDuration(op_pcm_total(l_ptrOpusFile, -1) / pMetadata->getSampleRate());

// If we don't have new enough Taglib we use libopusfile parser!
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
    TagLib::Ogg::Opus::File f(qBAFilename.constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        readXiphComment(pMetadata, *xiph);
    } else {
        // fallback
        const TagLib::Tag *tag(f.tag());
        if (tag) {
            readTag(pMetadata, *tag);
        } else {
            return ERR;
        }
    }
#else
    // From Taglib 1.9.x Opus is supported
    // Before that we have parse tags by this code
    int i = 0;
    const OpusTags *l_ptrOpusTags = op_tags(l_ptrOpusFile, -1);

    // This is left for debug reasons !!
    // qDebug() << "opus: We have " << l_ptrOpusTags->comments;
    for (i = 0; i < l_ptrOpusTags->comments; ++i) {
        QString l_SWholeTag = QString(l_ptrOpusTags->user_comments[i]);
        QString l_STag = l_SWholeTag.left(l_SWholeTag.indexOf("="));
        QString l_SPayload = l_SWholeTag.right((l_SWholeTag.length() - l_SWholeTag.indexOf("=")) - 1);

        if (!l_STag.compare("ARTIST")) {
            pMetadata->setArtist(l_SPayload);
        } else if (!l_STag.compare("ALBUM")) {
            pMetadata->setAlbum(l_SPayload);
        } else if (!l_STag.compare("BPM")) {
            pMetadata->setBpm(l_SPayload.toFloat());
        } else if (!l_STag.compare("YEAR") || !l_STag.compare("DATE")) {
            pMetadata->setYear(l_SPayload);
        } else if (!l_STag.compare("GENRE")) {
            pMetadata->setGenre(l_SPayload);
        } else if (!l_STag.compare("TRACKNUMBER")) {
            pMetadata->setTrackNumber(l_SPayload);
        } else if (!l_STag.compare("COMPOSER")) {
            pMetadata->setComposer(l_SPayload);
        } else if (!l_STag.compare("ALBUMARTIST")) {
            pMetadata->setAlbumArtist(l_SPayload);
        } else if (!l_STag.compare("TITLE")) {
            pMetadata->setTitle(l_SPayload);
        } else if (!l_STag.compare("REPLAYGAIN_TRACK_PEAK")) {
        } else if (!l_STag.compare("REPLAYGAIN_TRACK_GAIN")) {
            pMetadata->setReplayGainString (l_SPayload);
        } else if (!l_STag.compare("REPLAYGAIN_ALBUM_PEAK")) {
        } else if (!l_STag.compare("REPLAYGAIN_ALBUM_GAIN")) {
        }

        // This is left fot debug reasons!!
        //qDebug() << "Comment" << i << l_ptrOpusTags->comment_lengths[i] <<
        //" (" << l_ptrOpusTags->user_comments[i] << ")" << l_STag << "*" << l_SPayload;
    }
#endif

    return OK;
}

QImage SoundSourceOpus::parseCoverArt() {
#if (TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9))
    TagLib::Ogg::Opus::File f(getFilename().toLocal8Bit().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::readXiphCommentCover(*xiph);
    }
#endif
    return QImage();
}
