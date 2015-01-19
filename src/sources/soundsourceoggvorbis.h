/***************************************************************************
 soundsourceoggvorbis.h  -  ogg vorbis decoder
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

#ifndef SOUNDSOURCEOGGVORBIS_H
#define SOUNDSOURCEOGGVORBIS_H

#include "sources/soundsource.h"

class SoundSourceOggVorbis: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOggVorbis(QUrl url);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
