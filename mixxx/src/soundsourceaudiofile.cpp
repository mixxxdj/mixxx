/***************************************************************************
                          soundsourceaudiofile.cpp  -  description
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

#include "soundsourceaudiofile.h"
/*
  Class for reading files using libaudiofile
*/
SoundSourceAudioFile::SoundSourceAudioFile(const char* filename)
{
    fh = afOpenFile(filename,"r",0);
    if (fh == AF_NULL_FILEHANDLE) {
        qDebug("libaudiofile: Error opening file.");
        filelength = 0;
    } else
        filelength = 2*afGetFrameCount(fh,AF_DEFAULT_TRACK);

    channels = 2;
    type = "wav file.";
//    qDebug("length: %i",filelength);
}

SoundSourceAudioFile::~SoundSourceAudioFile()
{
    afCloseFile(fh);
};

long SoundSourceAudioFile::seek(long filepos)
{
    afSeekFrame(fh, AF_DEFAULT_TRACK, (AFframecount) (filepos/channels));
    return filepos;
}

/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceAudioFile::read(unsigned long size, const SAMPLE* destination)
{
    return afReadFrames(fh,AF_DEFAULT_TRACK, (SAMPLE *)destination,
            size/channels)*channels;
}

/*
  Return the length of the file in samples.
*/
inline long unsigned SoundSourceAudioFile::length()
{
    return filelength;
}

