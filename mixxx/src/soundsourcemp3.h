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

#include <qobject.h>
#include "defs.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef WIN64
  #define FPM_64BIT // So mad.h doesn't try to use inline assembly which MSVC-x64 doesn't support
#endif
#include "mad.h"

#include "errno.h"
#include "soundsource.h"
#include <sys/types.h>
#include <qfile.h>
//#include <sys/stat.h>
//#include <unistd.h>
#include <id3tag.h>
#include <q3ptrlist.h>

/** Struct used to store mad frames for seeking */
typedef struct MadSeekFrameType {
    unsigned char *m_pStreamPos;
    long int pos;
} MadSeekFrameType;


/**
  *@author Tue and Ken Haste Andersen
  */
  
class SoundSourceMp3 : public SoundSource {
public:
    SoundSourceMp3(QString qFilename);
    ~SoundSourceMp3();
    int open();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    unsigned long discard(unsigned long size);
    /** Return the length of the file in samples. */
    inline long unsigned length();
    int parseHeader();
    static QList<QString> supportedFileExtensions();
    
private:
    /** Returns the position of the frame which was found. The found frame is set to
      * the current element in m_qSeekList */
    int findFrame(int pos);
    /** Fills the string str with the content of the id3tag frame of frameid.
      * If the frame does not exist or is empty the string is left untouched. */
    static void getField(id3_tag *tag, const char *frameid, QString *str);
    /** Scale the mad sample to be in 16 bit range. */
    inline signed int madScale (mad_fixed_t sample);

    FILE *file;
    int bitrate;
    int framecount;
    int currentframe;
    /** current play position. */
    mad_timer_t pos;
    mad_timer_t filelength;
    mad_stream *Stream;
    mad_frame *Frame;
    mad_synth *Synth;
    unsigned inputbuf_len;
    char *inputbuf;
    /** Start index in Synth buffer of samples left over from previous call to read */
    int rest;
    /** Number of channels in file */
    int m_iChannels;

    /** It is not possible to make a precise seek in an mp3 file without decoding the whole stream.
      * To have precise seek within a limited range from the current decode position, we keep track
      * of past decodeded frame, and their exact position. If a seek occours and it is within the
      * range of frames we keep track of a precise seek occours, otherwise an unprecise seek is performed
      */
    Q3PtrList<MadSeekFrameType> m_qSeekList;
    /** Average frame size used when searching for a frame*/
    int m_iAvgFrameSize;
};


#endif
