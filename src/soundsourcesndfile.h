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
#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>


class SoundSourceSndFile : public Mixxx::SoundSource
{
public:
    explicit SoundSourceSndFile(QString qFilename);
    ~SoundSourceSndFile();
    Result open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    Result parseHeader();
    QImage parseCoverArt();
    static QList<QString> supportedFileExtensions();

private:
    SNDFILE *fh;
    SF_INFO info;
    int channels;
    unsigned long filelength;
};

#endif
