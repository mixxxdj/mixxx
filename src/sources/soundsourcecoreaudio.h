/**
 * \file soundsourcecoreaudio.h
 * \class SoundSourceCoreAudio
 * \brief Decodes M4As (etc) using the AudioToolbox framework included as
 *        part of Core Audio on OS X (and iOS).
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

#ifndef SOUNDSOURCECOREAUDIO_H
#define SOUNDSOURCECOREAUDIO_H

#include "sources/soundsource.h"

class SoundSourceCoreAudio : public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceCoreAudio(QString fileName);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif // ifndef SOUNDSOURCECOREAUDIO_H
