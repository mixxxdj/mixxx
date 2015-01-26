/***************************************************************************
 soundsourcemp3.cpp  -  description
 -------------------
 copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#include "sources/soundsourcemp3.h"

#include "sources/audiosourcemp3.h"
#include "metadata/trackmetadatataglib.h"

#include <taglib/mpegfile.h>

QList<QString> SoundSourceMp3::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("mp3");
    return list;
}

SoundSourceMp3::SoundSourceMp3(QUrl url)
        : SoundSource(url, "mp3") {
}

Result SoundSourceMp3::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    TagLib::MPEG::File f(getLocalFileNameBytes().constData());

    if (!readAudioProperties(pMetadata, f)) {
        return ERR;
    }

    // Now look for MP3 specific metadata (e.g. BPM)
    TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
    if (id3v2) {
        readID3v2Tag(pMetadata, *id3v2);
    } else {
        TagLib::APE::Tag *ape = f.APETag();
        if (ape) {
            readAPETag(pMetadata, *ape);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
    }

    return OK;
}

QImage SoundSourceMp3::parseCoverArt() const {
    QImage coverArt;
    TagLib::MPEG::File f(getLocalFileNameBytes().constData());
    TagLib::ID3v2::Tag* id3v2 = f.ID3v2Tag();
    if (id3v2) {
        coverArt = Mixxx::readID3v2TagCover(*id3v2);
    }
    if (coverArt.isNull()) {
        TagLib::APE::Tag *ape = f.APETag();
        if (ape) {
            coverArt = Mixxx::readAPETagCover(*ape);
        }
    }
    return coverArt;
}

Mixxx::AudioSourcePointer SoundSourceMp3::open() const {
    return Mixxx::AudioSourceMp3::create(getUrl());
}
