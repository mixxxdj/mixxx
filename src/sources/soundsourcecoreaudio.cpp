/**
 * \file soundsource.cpp
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Dec 12, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sources/soundsourcecoreaudio.h"

#include "sources/audiosourcecoreaudio.h"
#include "trackmetadatataglib.h"

#include <taglib/mpegfile.h>
#include <taglib/mp4file.h>

#include <QtDebug>

QList<QString> SoundSourceCoreAudio::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp3");
    list.push_back("mp2");
    //Can add mp3, mp2, ac3, and others here if you want.
    //See:
    //  http://developer.apple.com/library/mac/documentation/MusicAudio/Reference/AudioFileConvertRef/Reference/reference.html#//apple_ref/doc/c_ref/AudioFileTypeID

    //XXX: ... but make sure you implement handling for any new format in ParseHeader!!!!!! -- asantoni
    return list;
}

SoundSourceCoreAudio::SoundSourceCoreAudio(QString fileName)
        : Super(fileName) {
}

Result SoundSourceCoreAudio::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    if (getType() == "m4a") {
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            readMP4Tag(pMetadata, *mp4);
        } else {
            // fallback
            const TagLib::Tag *tag(f.tag());
            if (tag) {
                readTag(pMetadata, *tag);
            } else {
                return ERR;
            }
        }
    } else if (getType() == "mp3") {
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
        if (!readAudioProperties(pMetadata, f)) {
            return ERR;
        }
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
    } else if (getType() == "mp2") {
        //TODO: MP2 metadata. Does anyone use mp2 files anymore?
        //      Feels like 1995 again...
        return ERR;
    }

    return OK;
}

QImage SoundSourceCoreAudio::parseCoverArt() const {
    QImage coverArt;
    if (getType() == "m4a") {
        TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
        TagLib::MP4::Tag *mp4(f.tag());
        if (mp4) {
            return Mixxx::readMP4TagCover(*mp4);
        } else {
            return QImage();
        }
    } else if (getType() == "mp3") {
        TagLib::MPEG::File f(getFilename().toLocal8Bit().constData());
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
    return coverArt;
}

Mixxx::AudioSourcePointer SoundSourceCoreAudio::open() const {
    return Mixxx::AudioSourceCoreAudio::open(getFilename());
}
