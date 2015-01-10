/***************************************************************************
 soundsourceoggvorbis.cpp  -  ogg vorbis decoder
 -------------------
 copyright            : (C) 2003 by Svein Magne Bang
 email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sources/soundsourceoggvorbis.h"

#include "sources/audiosourceoggvorbis.h"
#include "metadata/trackmetadatataglib.h"

#include <taglib/vorbisfile.h>

QList<QString> SoundSourceOggVorbis::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("ogg");
    return list;
}

SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename)
        : SoundSource(qFilename, "ogg") {
}

/*
 Parse the the file to get metadata
 */
Result SoundSourceOggVorbis::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    TagLib::Ogg::Vorbis::File f(getFilename().toLocal8Bit().constData());

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
    TagLib::Ogg::Vorbis::File f(getFilename().toLocal8Bit().constData());
    TagLib::Ogg::XiphComment *xiph = f.tag();
    if (xiph) {
        return Mixxx::readXiphCommentCover(*xiph);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceOggVorbis::open() const {
    return Mixxx::AudioSourceOggVorbis::create(getFilename());
}
