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

SoundSourceAudioFile::SoundSourceAudioFile(QString qFilename) : SoundSource(qFilename)
{
    fh = afOpenFile( qFilename.latin1() ,"r",0);
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
}

SoundSourceAudioFile::~SoundSourceAudioFile()
{
    afCloseFile(fh);
    if (buffer)
        delete [] buffer;
};

long SoundSourceAudioFile::seek(long filepos)
{
//    if (filelength>0)
    {
//	   qDebug("seek %i, len %i, channels %i",filepos,filelength,channels);
//        filepos = max(0, min(filepos,filelength));
    
        if (afSeekFrame(fh, AF_DEFAULT_TRACK, (AFframecount) (filepos/2))<0)
            qDebug("libaudiofile: Seek ERR.");
        return filepos;
    }
//    return 0;
}

/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceAudioFile::read(unsigned long size, const SAMPLE* destination)
{
    SAMPLE *dest = (SAMPLE *)destination;
    if (channels==2)
    {    
	//qDebug("req %i, ch %i, frames %i",size,channels,size/channels);
        int readNo = afReadFrames(fh,AF_DEFAULT_TRACK, dest, size/channels);
	//qDebug("read  %i",readNo);
        return readNo*channels;
    }
    else
    {
        // If the file is not in stereo, make the returned buffer so.
        int readNo = afReadFrames(fh,AF_DEFAULT_TRACK, buffer, size*channels/2);
        int j=0;
        for (int i=0; i<readNo*channels; i+=channels)
        {
            dest[j] = buffer[i];
            ++j;
            if (channels>1)
                dest[j] = buffer[i+1];
            else
                dest[j] = buffer[i];
            ++j;
        }
        return readNo/channels*2;
    }
}

int SoundSourceAudioFile::ParseHeader(TrackInfoObject *Track)
{
    QString location = Track->getLocation();
    AFfilehandle fh = afOpenFile(location.latin1() , "r", 0);
    if (fh == AF_NULL_FILEHANDLE)
    {
        qDebug("libaudiofile: Error opening file.");
        return(ERR);
    }

    Track->setType(location.section(".",-1).lower());
    Track->setDuration((int)(afGetFrameCount(fh, AF_DEFAULT_TRACK)/afGetRate(fh, AF_DEFAULT_TRACK)));
    Track->setBitrate((int)((Track->getLength()/(Track->getDuration()*afGetRate(fh, AF_DEFAULT_TRACK))*afGetRate(fh, AF_DEFAULT_TRACK)*8.)/1000.));
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

