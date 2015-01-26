#include "sources/soundsourceopus.h"

#include "sources/audiosourceopus.h"
#include "metadata/trackmetadatataglib.h"

// TagLib has support for the Ogg Opus file format since version 1.9
#define TAGLIB_HAS_OPUSFILE \
    ((TAGLIB_MAJOR_VERSION > 1) || ((TAGLIB_MAJOR_VERSION == 1) && (TAGLIB_MINOR_VERSION >= 9)))

#if TAGLIB_HAS_OPUSFILE
#include <taglib/opusfile.h>
#endif

QList<QString> SoundSourceOpus::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("opus");
    return list;
}

SoundSourceOpus::SoundSourceOpus(QUrl url) :
        SoundSource(url, "opus") {
}

namespace {
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
}

/*
 Parse the file to get metadata
 */
Result SoundSourceOpus::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    const QByteArray qbaFilename(getLocalFileNameBytes());

// If we don't have new enough Taglib we use libopusfile parser!
#if TAGLIB_HAS_OPUSFILE
    TagLib::Ogg::Opus::File f(qbaFilename.constData());

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
    int error = 0;
    OggOpusFileOwner l_ptrOpusFile(
            op_open_file(qbaFilename.constData(), &error));

    // From Taglib 1.9.x Opus is supported
    // Before that we have parse tags by this code
    int i = 0;
    const OpusTags *l_ptrOpusTags = op_tags(l_ptrOpusFile, -1);

    pMetadata->setChannels(op_channel_count(l_ptrOpusFile, -1));
    pMetadata->setSampleRate(Mixxx::AudioSourceOpus::kFrameRate);
    pMetadata->setBitrate(op_bitrate(l_ptrOpusFile, -1) / 1000);
    pMetadata->setDuration(
            op_pcm_total(l_ptrOpusFile, -1) / pMetadata->getSampleRate());

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
            pMetadata->setReplayGainDbString (l_SPayload);
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

QImage SoundSourceOpus::parseCoverArt() const {
#if TAGLIB_HAS_OPUSFILE
    TagLib::Ogg::Opus::File f(getLocalFileNameBytes().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::readXiphCommentCover(*xiph);
    }
#endif
    return QImage();
}

Mixxx::AudioSourcePointer SoundSourceOpus::open() const {
    return Mixxx::AudioSourceOpus::create(getUrl());
}
