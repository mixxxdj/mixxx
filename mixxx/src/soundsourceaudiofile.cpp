/**************************************************************************
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
#include "trackinfoobject.h"

SoundSourceAudioFile::SoundSourceAudioFile( QString sFilename ) 
{
    fh = afOpenFile( sFilename.latin1() ,"r",0);
    if (fh == AF_NULL_FILEHANDLE) {
        qDebug("libaudiofile: Error opening file.");
        filelength = 0;
    } else
        filelength = 2*afGetFrameCount(fh,AF_DEFAULT_TRACK);

    channels = 2;
    SRATE = (int)afGetRate(fh,AF_DEFAULT_TRACK);
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
    return afReadFrames(fh,AF_DEFAULT_TRACK, (SAMPLE *)destination,size/channels)*channels;
}

void SoundSourceAudioFile::ParseHeader(TrackInfoObject *Track)
{
    QString location = Track->m_sFilepath+'/'+Track->m_sFilename;
    AFfilehandle fh = afOpenFile(location.ascii() , "r", 0);
    if (fh == AF_NULL_FILEHANDLE)
    {
        qDebug("libaudiofile: Error opening file.");
        return;
    }

    Track->m_sType = "wav";
    Track->m_sBitrate = QString("%1").arg(Track->m_iLength/(afGetRate(fh, AF_DEFAULT_TRACK)));
    Track->m_iDuration = Track->m_iLength/(4*afGetRate(fh, AF_DEFAULT_TRACK));
    qDebug("Parsed header: bitrate %s", Track->m_sBitrate);
    afCloseFile(fh);
}

/*
  Return the length of the file in samples.
*/
inline long unsigned SoundSourceAudioFile::length()
{
    return filelength;
}

