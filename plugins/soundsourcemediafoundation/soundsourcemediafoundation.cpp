/**
 * \file soundsourcemediafoundation.cpp
 * \author Bill Good <bkgood at gmail dot com>
 * \author Albert Santoni <alberts at mixxx dot org>
 * \date Jan 10, 2011
 * \note This file uses COM interfaces defined in Windows 7 and later added to
 * Vista and Server 2008 via the "Platform Update Supplement for Windows Vista
 * and for Windows Server 2008" (http://support.microsoft.com/kb/2117917).
 * Earlier versions of Vista (and possibly Server 2008) have some Media
 * Foundation interfaces but not the required IMFSourceReader, and are missing
 * the Microsoft-provided AAC decoder. XP does not include Media Foundation.
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundsourcemediafoundation.h"
#include "audiosourcemediafoundation.h"

#include "metadata/trackmetadatataglib.h"

#include <taglib/mp4file.h>

#include <QtDebug>

// static
QList<QString> SoundSourceMediaFoundation::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp4");
    return list;
}

SoundSourceMediaFoundation::SoundSourceMediaFoundation(QString fileName)
        : SoundSource(fileName, "m4a") {
}

Result SoundSourceMediaFoundation::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    // Must be toLocal8Bit since Windows fopen does not do UTF-8
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

    return OK;
}

QImage SoundSourceMediaFoundation::parseCoverArt() const {
    TagLib::MP4::File f(getFilename().toLocal8Bit().constData());
    TagLib::MP4::Tag *mp4(f.tag());
    if (mp4) {
        return Mixxx::readMP4TagCover(*mp4);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceMediaFoundation::open() const {
    return Mixxx::AudioSourceMediaFoundation::open(getFilename());
}
