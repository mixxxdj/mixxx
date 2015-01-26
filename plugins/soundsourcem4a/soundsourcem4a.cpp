/***************************************************************************
 soundsourcem4a.cpp  -  mp4/m4a decoder
 -------------------
 copyright            : (C) 2008 by Garth Dahlstrom
 email                : ironstorm@users.sf.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "soundsourcem4a.h"
#include "audiosourcem4a.h"

#include "metadata/trackmetadatataglib.h"

#include <taglib/mp4file.h>

namespace Mixxx {

QList<QString> SoundSourceM4A::supportedFileExtensions() {
    QList<QString> list;
    list.push_back("m4a");
    list.push_back("mp4");
    return list;
}

SoundSourceM4A::SoundSourceM4A(QUrl url)
        : SoundSourcePlugin(url, "m4a") {
}

Result SoundSourceM4A::parseMetadata(Mixxx::TrackMetadata* pMetadata) const {
    TagLib::MP4::File f(getLocalFileNameBytes().constData());

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

QImage SoundSourceM4A::parseCoverArt() const {
    TagLib::MP4::File f(getLocalFileNameBytes().constData());
    TagLib::MP4::Tag *mp4(f.tag());
    if (mp4) {
        return readMP4TagCover(*mp4);
    } else {
        return QImage();
    }
}

Mixxx::AudioSourcePointer SoundSourceM4A::open() const {
    return Mixxx::AudioSourceM4A::create(getUrl());
}

} // namespace Mixxx

extern "C" MY_EXPORT const char* getMixxxVersion() {
    return VERSION;
}

extern "C" MY_EXPORT int getSoundSourceAPIVersion() {
    return MIXXX_SOUNDSOURCE_API_VERSION;
}

extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName) {
    return new Mixxx::SoundSourceM4A(fileName);
}

extern "C" MY_EXPORT char** supportedFileExtensions() {
    const QList<QString> supportedFileExtensions(
            Mixxx::SoundSourceM4A::supportedFileExtensions());
    return Mixxx::SoundSourcePlugin::allocFileExtensions(
            supportedFileExtensions);
}

extern "C" MY_EXPORT void freeFileExtensions(char** fileExtensions) {
    Mixxx::SoundSourcePlugin::freeFileExtensions(fileExtensions);
}
