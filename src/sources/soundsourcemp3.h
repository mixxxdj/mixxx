/***************************************************************************
                          soundsourcemp3.h  -  description
                             -------------------
    begin                : Wed Feb 20 2002
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

#ifndef SOUNDSOURCEMP3_H
#define SOUNDSOURCEMP3_H

#include "sources/soundsource.h"

/**
  *@author Tue and Ken Haste Andersen
  */

class SoundSourceMp3 : public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceMp3(QString qFilename);

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
};

#endif
