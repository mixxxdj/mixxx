/***************************************************************************
                          soundsourceheavymp3.h  -  description
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

#ifndef SOUNDSOURCEHEAVYMP3_H
#define SOUNDSOURCEHEAVYMP3_H

#include <qobject.h>
#include "defs.h"
#include "soundsource.h"
#include <mad.h>
#include "errno.h"
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class SoundSourceHeavymp3 : public SoundSource {
 private:
	 std::vector<SAMPLE> buffer;
    long unsigned bufferlen;
    FILE *file;
    unsigned inputbuf_len;
    unsigned char *inputbuf;
    int bitrate;
    long position;
 public:
    SoundSourceHeavymp3(const char*);
    ~SoundSourceHeavymp3();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    long unsigned length();
};


#endif
