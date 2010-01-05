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

#include "trackinfoobject.h"
#include "soundsourcesndfile.h"
#include <qstring.h>
#include <QtDebug>
//Added by qt3to4:
#include <Q3ValueList>
/*
   Class for reading files using libsndfile
 */
SoundSourceSndFile::SoundSourceSndFile(QString qFilename) : SoundSource(qFilename)
{
    info = new SF_INFO;
    filelength = 0;
    QByteArray qbaFilename = qFilename.toUtf8();
    fh = sf_open( qbaFilename.data(), SFM_READ, info );
    if (fh == 0 || !sf_format_check(info))
    {
        qDebug() << "libsndfile: Error opening file" << qFilename;
        return;
    }

    channels = info->channels;

    filelength = 2*info->frames; // File length with two interleaved channels
    SRATE =  info->samplerate;
}

SoundSourceSndFile::~SoundSourceSndFile()
{
    if (filelength > 0)
        sf_close(fh);
    delete info;
}

long SoundSourceSndFile::seek(long filepos)
{
    unsigned long filepos2 = (unsigned long)filepos;
    if (filelength>0)
    {
        filepos2 = math_min(filepos2,filelength);
        sf_seek(fh, (sf_count_t)filepos2/2, SEEK_SET);
        //Note that we don't error check sf_seek because it reports
        //benign errors under normal usage (ie. we sometimes seek past the end
        //of a song, and it will stop us.)
        return filepos2;
    }
    return 0;
}

/*
   read <size> samples into <destination>, and return the number of
   samples actually read. A sample is a single float representing a
   sample on one channel of the audio. In the case of a monaural file
   then size/2 samples are read from the mono file, and they are
   doubled into stereo.
 */
unsigned SoundSourceSndFile::read(unsigned long size, const SAMPLE * destination)
{
    SAMPLE * dest = (SAMPLE *)destination;
    if (filelength > 0)
    {
        if (channels==2)
        {
            unsigned long no = sf_read_short(fh, dest, size);

            // rryan 2/2009 This code used to lie and say we read
            // 'size' samples no matter what. I left this array
            // zeroing code here in case the Reader doesn't check
            // against this.
            for (unsigned long i=no; i<size; ++i)
                dest[i] = 0;

            return no;
        }
        else if(channels==1)
        {
            // We are not dealing with a stereo file. Read fewer
            // samples than requested and double them because we
            // pretend to every reader that all files are in stereo.
            int readNo = sf_read_short(fh, dest, size/2);

            // readNo*2 is strictly less than available buffer space

            // rryan 2/2009
            // Mini-proof of the below:
            // size = 20, destination is a 20 element array 0-19
            // readNo = 10 (or less, but 10 in this case)
            // i = 10-1 = 9, so dest[9*2] and dest[9*2+1],
            // so the first iteration touches the very ends of destination
            // on the last iteration, dest[0] and dest[1] are assigned to dest[0]
            
            for(int i=(readNo-1); i>=0; i--) {
                dest[i*2]     = dest[i];
                dest[(i*2)+1] = dest[i];
            }

            // We doubled the readNo bytes we read into stereo.
            return readNo * 2;
        } else {
            // We do not support music with more than 2 channels.
            return 0;
        }
    }

    // The file has errors or is not open. Tell the truth and return 0.
    return 0;
}

int SoundSourceSndFile::ParseHeader( TrackInfoObject * Track )
{
    SF_INFO info;
    QString location = Track->getLocation();
    QByteArray qbaLocation = location.toUtf8();
    SNDFILE * fh = sf_open(qbaLocation.data() ,SFM_READ, &info);
    //const char* err = sf_strerror(0);
    if (fh == 0 || !sf_format_check(&info))
    {
        qDebug() << "libsndfile: ERR opening file.";
        return ERR;
    }

    Track->setType(location.section(".",-1).toLower());
    Track->setBitrate((int)(info.samplerate*32./1000.));
    Track->setDuration(info.frames/info.samplerate);
    Track->setSampleRate(info.samplerate);
    Track->setChannels(info.channels);

    const char *string;
    string = sf_get_string(fh, SF_STR_ARTIST);
//    qDebug() << location << "SF_STR_ARTIST" << string;
    if(string && strlen(string))
        Track->setArtist(string);
    string = sf_get_string(fh, SF_STR_TITLE);
    if(string && strlen(string))
        Track->setTitle(string);
//    qDebug() << location << "SF_STR_TITLE" << string;
    string = sf_get_string(fh, SF_STR_DATE);
    if (string && strlen(string))
        Track->setYear(string);
    
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

Q3ValueList<long> * SoundSourceSndFile::getCuePoints()
{
    QByteArray qbaFilename = m_qFilename.toUtf8();

    // Ensure that the file ends with ".wav"
    if (!m_qFilename.endsWith(".wav"))
        return 0;

    // Open file
    FILE * fh  = fopen(qbaFilename.data(), "r");

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

    // Allocate cue point list
    Q3ValueList<long> *pCueList = new Q3ValueList<long>;

    // Read each cue point
    for (int i=0; i<no; ++i)
    {
        if (feof(fh))
            return 0;

        // Seek to actual cue point data
        fseek(fh, 5*4,SEEK_CUR);

        long cuepoint;
        fread(&cuepoint, sizeof(long), 1, fh);

        pCueList->append(cuepoint*channels);
    }

    return pCueList;
}
