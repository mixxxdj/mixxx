/***************************************************************************
                          soundsourcem4a.h  -  mp4/m4a decoder
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

#ifndef SOUNDSOURCEM4A_H
#define SOUNDSOURCEM4A_H

#include "sources/soundsource.h"
#include "defs_version.h"

//As per QLibrary docs: http://doc.trolltech.com/4.6/qlibrary.html#resolve
#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif


namespace Mixxx {

class SoundSourceM4A : public SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceM4A(QString fileName);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

} // namespace Mixxx

extern "C" MY_EXPORT const char* getMixxxVersion();
extern "C" MY_EXPORT int getSoundSourceAPIVersion();
extern "C" MY_EXPORT Mixxx::SoundSource* getSoundSource(QString fileName);
extern "C" MY_EXPORT char** supportedFileExtensions();
extern "C" MY_EXPORT void freeFileExtensions(char **exts);

#endif
