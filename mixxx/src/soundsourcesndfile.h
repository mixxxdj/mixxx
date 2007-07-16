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
//Added by qt3to4:
#include <Q3ValueList>
class TrackInfoObject;

class SoundSourceSndFile : public SoundSource 
{
public:
    SoundSourceSndFile(QString qFilename);
    ~SoundSourceSndFile();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    static int ParseHeader( TrackInfoObject * );
    Q3ValueList<long> *getCuePoints();

private:
    int channels;
    SNDFILE *fh;
    SF_INFO *info;
    unsigned long filelength;
    SAMPLE *buffer;
};

#endif
