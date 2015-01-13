/**
 * \file sourdsourceflac.h
 * \class SoundSourceFLAC
 * \brief Decodes FLAC files using libFLAC for Mixxx.
 * \author Bill Good <bkgood at gmail dot com>
 * \date May 22, 2010
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUNDSOURCEFLAC_H
#define SOUNDSOURCEFLAC_H

#include "sources/soundsource.h"

class SoundSourceFLAC: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFLAC(QString fileName);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif // ifndef SOUNDSOURCEFLAC_H
