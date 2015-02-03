#include "sources/soundsourceoggvorbis.h"

#include "sources/audiosourceoggvorbis.h"
#include "metadata/trackmetadatataglib.h"

#include <taglib/vorbisfile.h>

QList<QString> SoundSourceOggVorbis::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("ogg");
    return list;
}

SoundSourceOggVorbis::SoundSourceOggVorbis(QUrl url) :
        SoundSource(url, "ogg") {
}

/*
 Parse the the file to get metadata
 */
Result SoundSourceOggVorbis::parseMetadata(
        Mixxx::TrackMetadata* pMetadata) const {
    TagLib::Ogg::Vorbis::File f(getLocalFileNameBytes().constData());

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

    return OK;
}

QImage SoundSourceOggVorbis::parseCoverArt() const {
    TagLib::Ogg::Vorbis::File f(getLocalFileNameBytes().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::readXiphCommentCover(*xiph);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceOggVorbis::open() const {
    return Mixxx::AudioSourceOggVorbis::create(getUrl());
}
