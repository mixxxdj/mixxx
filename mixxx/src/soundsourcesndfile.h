/***************************************************************************
                          soundsourcesndfile.h  -  description
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

#ifndef SOUNDSOURCESNDFILE_H
#define SOUNDSOURCESNDFILE_H

#include "soundsource.h"
#include <stdio.h>
#include <sndfile.h>

class SoundSourceSndFile : public Mixxx::SoundSource
{
public:
    SoundSourceSndFile(QString qFilename);
    ~SoundSourceSndFile();
    int open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();

private:
    bool m_bOpened;
    int channels;
    SNDFILE *fh;
    SF_INFO *info;
    unsigned long filelength;
};

#endif
