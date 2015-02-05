#include "sources/soundsourceopus.h"

#include "sources/audiosourceopus.h"
#include "metadata/trackmetadatataglib.h"

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
    if (OK == readTrackMetadataFromFile(pMetadata, getLocalFileName())) {
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

    int error = 0;
    OggOpusFileOwner l_ptrOpusFile(
            op_open_file(getLocalFileNameBytes().constData(), &error));

    int i = 0;
    const OpusTags *l_ptrOpusTags = op_tags(l_ptrOpusFile, -1);

    pMetadata->setChannels(op_channel_count(l_ptrOpusFile, -1));
    pMetadata->setSampleRate(Mixxx::AudioSourceOpus::kFrameRate);
    pMetadata->setBitrate(op_bitrate(l_ptrOpusFile, -1) / 1000);
    pMetadata->setDuration(
            op_pcm_total(l_ptrOpusFile, -1) / pMetadata->getSampleRate());

    bool hasDate = false;
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
        } else if (!l_STag.compare("DATE")) {
            // Prefer "DATE" over "YEAR"
            pMetadata->setYear(l_SPayload.trimmed());
            // Avoid to overwrite "DATE" with "YEAR"
            hasDate |= !pMetadata->getYear().isEmpty();
        } else if (!hasDate && !l_STag.compare("YEAR")) {
            pMetadata->setYear(l_SPayload.trimmed());
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
        } else if (!l_STag.compare("REPLAYGAIN_TRACK_GAIN")) {
            pMetadata->setReplayGain(Mixxx::TrackMetadata::parseReplayGain(l_SPayload));
        }

        // This is left fot debug reasons!!
        //qDebug() << "Comment" << i << l_ptrOpusTags->comment_lengths[i] <<
        //" (" << l_ptrOpusTags->user_comments[i] << ")" << l_STag << "*" << l_SPayload;
    }

    return OK;
}

Mixxx::AudioSourcePointer SoundSourceOpus::open() const {
    return Mixxx::AudioSourceOpus::create(getUrl());
}
