/***************************************************************************
                          soundsourcesndfile.cpp  -  description
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

#include "soundsourcesndfile.h"
#include <qstring.h>
#include "trackinfoobject.h"
/*
  Class for reading files using libsndfile
*/
SoundSourceSndFile::SoundSourceSndFile( QString sFilename )
{
    info = new SF_INFO;
    fh = sf_open( sFilename.latin1(), SFM_READ, info );
    if (fh == 0 || !sf_format_check(info))
    {
        qDebug("libsndfile: ERR opening file.");
        filelength = 0;
        return;
    } else
        filelength = 2*info->frames;

    if (info->channels != 2)
    {
        qDebug("libsndfile: Only two-channel files are supported.");
        sf_close(fh);
        filelength = 0;
        return;
    }        
    channels = 2;
    SRATE =  info->samplerate;
    
    type = "wav file.";
}

SoundSourceSndFile::~SoundSourceSndFile()
{
    if (filelength > 0)
        sf_close(fh);
    delete info;
};

long SoundSourceSndFile::seek(long filepos)
{
    if (filelength > 0)
        if (sf_seek(fh, (sf_count_t)filepos, SEEK_SET) == -1)
            qDebug("libsndfile: Seek ERR.");
    
    return filepos;
}

/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceSndFile::read(unsigned long size, const SAMPLE* destination)
{
    if (filelength > 0)
        return sf_read_short(fh,(SAMPLE *)destination, size);
    else
    {
        for (unsigned int i=0; i<size; i++)
            ((SAMPLE *)destination)[i] = 0;
        return size;
    }
}

int SoundSourceSndFile::ParseHeader( TrackInfoObject *Track )
{
    SF_INFO info;
    QString location = Track->m_sFilepath+'/'+Track->m_sFilename;
    SNDFILE *fh = sf_open( location.ascii() ,SFM_READ, &info);
    if (fh == 0 || !sf_format_check(&info))
    {
        qDebug("libsndfile: ERR opening file.");
        return ERR;
    }

    Track->m_sType = "wav";
    Track->m_sBitrate = QString("%1").arg((int)(info.samplerate*16./1000.));
    Track->m_iDuration = Track->m_iLength/(4*info.samplerate);

    sf_close( fh );
    return OK;
}

/*
  Return the length of the file in samples.
*/
inline long unsigned SoundSourceSndFile::length()
{
    return filelength;
}

