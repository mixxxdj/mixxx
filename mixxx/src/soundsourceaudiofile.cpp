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
        channels = 2;
    }
    else
        channels = afGetChannels(fh, AF_DEFAULT_TRACK);

    // Buffer only used when opening a non-stereo file
    if (channels!=2)
        buffer = new SAMPLE[MAX_BUFFER_LEN];
    else
        buffer = 0;
        
    filelength = channels*afGetFrameCount(fh,AF_DEFAULT_TRACK);

    SRATE = (int)afGetRate(fh,AF_DEFAULT_TRACK);
    type = "wav file.";
//    qDebug("length: %i",filelength);
}

SoundSourceAudioFile::~SoundSourceAudioFile()
{
    afCloseFile(fh);
    if (buffer)
        delete [] buffer;
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
    if (channels==2)
        return afReadFrames(fh,AF_DEFAULT_TRACK, (SAMPLE *)destination, size/channels)*channels;
    else
    {
        // If the file is not in stereo, make the returned buffer.
        int readNo = afReadFrames(fh,AF_DEFAULT_TRACK, buffer, size/2);
        for (int i=0; i<readNo; i+=channels)
        {
            for (int j=0; j<2; j++)
                (SAMPLE *)destination[(i*max(channels,2))+j] = buffer[(i*channels)+j];
        }
        return readNo*2;
    }
}

int SoundSourceAudioFile::ParseHeader(TrackInfoObject *Track)
{
    QString location = Track->m_sFilepath+'/'+Track->m_sFilename;
    AFfilehandle fh = afOpenFile(location.ascii() , "r", 0);
    if (fh == AF_NULL_FILEHANDLE)
    {
        qDebug("libaudiofile: Error opening file.");
        return(ERR);
    }

    Track->m_sType = "wav";
    Track->m_iDuration = (int)(afGetFrameCount(fh, AF_DEFAULT_TRACK)/afGetRate(fh, AF_DEFAULT_TRACK));
    Track->m_sBitrate = QString("%1").arg((int)((Track->m_iLength/(Track->m_iDuration*afGetRate(fh, AF_DEFAULT_TRACK))*
                                                 afGetRate(fh, AF_DEFAULT_TRACK)*8.)/1000.));
    afCloseFile(fh);
    return OK;
}

/*
  Return the length of the file in samples.
*/
inline long unsigned SoundSourceAudioFile::length()
{
    return filelength;
}

