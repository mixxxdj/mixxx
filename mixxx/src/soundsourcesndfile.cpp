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
SoundSourceSndFile::SoundSourceSndFile(QString qFilename) : SoundSource(qFilename)
{
    info = new SF_INFO;
    fh = sf_open( qFilename.latin1(), SFM_READ, info );
    if (fh == 0 || !sf_format_check(info))
    {
        qDebug("libsndfile: Error opening file %s",qFilename.latin1());
        filelength = 0;
        return;
    }

    channels = info->channels;

    // Buffer only used when opening a non-stereo file
    if (channels!=2)
        buffer = new SAMPLE[MAX_BUFFER_LEN];
    else
        buffer = 0;

    filelength = 2*info->frames; // File length with two interleaved channels
    SRATE =  info->samplerate;
}

SoundSourceSndFile::~SoundSourceSndFile()
{
    if (filelength > 0)
        sf_close(fh);
    delete info;
    if (buffer)
        delete [] buffer;
};

long SoundSourceSndFile::seek(long filepos)
{
    unsigned long filepos2 = (unsigned long)filepos;
    if (filelength>0)
    {
        filepos2 = min(filepos2,filelength);
        if (sf_seek(fh, (sf_count_t)filepos2/2, SEEK_SET) == -1)
            qDebug("libsndfile: Seek ERR.");
        return filepos2;
    }
    return 0;
}

/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceSndFile::read(unsigned long size, const SAMPLE* destination)
{
    SAMPLE *dest = (SAMPLE *)destination;
    if (filelength > 0)
    {
        if (channels==2)
        {
            unsigned long no = sf_read_short(fh, dest, size);
            for (unsigned long i=no; i<size; ++i)
                dest[i] = 0;
            return size;
        }
        else
        {
            // If the file is not in stereo, make the returned buffer so.
            int readNo = sf_read_short(fh, buffer, channels*size/2);
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
            return 2*readNo/channels;
        }
    }
    else
    {
        for (unsigned int i=0; i<size; i++)
            ((SAMPLE *)destination)[i] = 0;
    }
    return size;
}

int SoundSourceSndFile::ParseHeader( TrackInfoObject *Track )
{
    SF_INFO info;
    QString location = Track->getLocation();
    SNDFILE *fh = sf_open( location.latin1() ,SFM_READ, &info);
    if (fh == 0 || !sf_format_check(&info))
    {
        qDebug("libsndfile: ERR opening file.");
        return ERR;
    }

    Track->setType(location.section(".",-1).lower());
    Track->setBitrate( (int)(info.samplerate*32./1000.) );
    Track->setDuration( info.frames/info.samplerate );

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

QPtrList<long unsigned int> *SoundSourceSndFile::getCuePoints()
{
    // Ensure that the file ends with ".wav"
    if (!m_qFilename.endsWith(".wav"))
        return 0;
        
    // Open file
    FILE *fh  = fopen(m_qFilename.latin1(),"r");

    // Check the file magic header bytes
    char str[4];
    fread(&str, sizeof(char), 4, fh);
    if (!strncmp(str, "RIFF", 4)==0)
        return 0;
    long no;
    fread(&no, sizeof(long), 1, fh);
    fread(&str, sizeof(char), 4, fh);
    if (!strncmp(str, "WAVE", 4)==0)
        return 0;

    // Skip to the "cue " section, ignoring everything else
    while (1)
    {
        if (feof(fh))
            return 0;
        
        fread(&str, sizeof(char), 4, fh);
        fread(&no, sizeof(long), 1, fh);
        if (strncmp(str, "cue ", 4)==0)
            break;    
        else if (strncmp(str, "data", 4)==0)
            return 0;
        else
            fseek(fh, no,SEEK_CUR);
    }    

    // Read number of cue points
    fread(&no, sizeof(long), 1, fh);
    if (no<1)
        return 0;
    
    // Read each cue point
    for (int i=0; i<no; ++i)
    {
        if (feof(fh))
            return 0;
    
        // Seek to actual cue point data
        fseek(fh, 5*4,SEEK_CUR);
        
        long cuepoint;
        fread(&cuepoint, sizeof(long), 1, fh);
    
        qDebug("cue point %i",cuepoint);
    }
        
    return 0;
}
