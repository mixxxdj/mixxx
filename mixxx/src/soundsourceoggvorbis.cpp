/***************************************************************************
                          soundsourceoggvorbis.cpp  -  ogg vorbis decoder
                             -------------------
    copyright            : (C) 2003 by Svein Magne Bang
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
#include "soundsourceoggvorbis.h"
#include <QtDebug>
#ifdef __WIN__
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __MACX__
#define OV_ENDIAN_ARG 1
#else
#ifdef __LINUX__
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define OV_ENDIAN_ARG 0
#else
#define OV_ENDIAN_ARG 1
#endif
#else
#define OV_ENDIAN_ARG 0
#endif
#endif

/*
   Class for reading Ogg Vorbis
 */

SoundSourceOggVorbis::SoundSourceOggVorbis(QString qFilename) 
: SoundSource(qFilename)
, dest (0)
, pRead(0)
{
    QByteArray qBAFilename = qFilename.toUtf8();
    vorbisfile =  fopen(qBAFilename.data(), "r");
    if (!vorbisfile)
    {
        qDebug() << "oggvorbis: cannot open" << qFilename;
        return;
    }

    // Apparently this is needed to make windows happy:
#ifdef __WIN__
    _setmode(_fileno(vorbisfile), _O_BINARY);
#endif

    if(ov_open(vorbisfile, &vf, NULL, 0) < 0)
    {
        qDebug() << "oggvorbis: Input does not appear to be an Ogg bitstream.";
        filelength = 0;
        return;
    }
    else
    {

        // extract metadata
        vorbis_info * vi=ov_info(&vf,-1);

        channels = vi->channels;
        SRATE = vi->rate;

        if(channels > 2){
            qDebug() << "oggvorbis: No support for more than 2 channels!";
            return;
        }

        filelength = (unsigned long) ov_pcm_total(&vf, -1) * 2;
    }
}

SoundSourceOggVorbis::~SoundSourceOggVorbis()
{
    if (filelength > 0){
        ov_clear(&vf);
        // note that fclose() is not needed, ov_clear() does this as well
    }
};


/*
   seek to <filepos>
 */

long SoundSourceOggVorbis::seek(long filepos)
{
    if (ov_seekable(&vf)){
        index = ov_pcm_seek(&vf, filepos/2);
        return filepos;
    } else{
        qDebug() << "ogg vorbis: Seek ERR.";
        return 0;
    }
}


/*
   read <size> samples into <destination>, and return the number of
   samples actually read.
 */

unsigned SoundSourceOggVorbis::read(volatile unsigned long size, const SAMPLE * destination)
{
    pRead  = (char*) destination;
    dest   = (SAMPLE*) destination;
    
    index  = ret = 0;
    needed = size*channels;

    // loop until requested number of samples has been retrieved
    while (needed > 0)
    {
        index  += ret;
        needed -= ret;

        // read samples into buffer
        ret = ov_read(&vf, pRead+index, needed, OV_ENDIAN_ARG, 2, 1, &current_section);
        
        // if eof (ret==0) or error(ret<0) we fill the rest with zero
        if (ret <= 0)
        {
            while (needed > 0)
            {
                pRead[index] = 0;
                index++;
                needed--;
            }
        }
    }

    // convert into stereo if file is mono
    if (channels == 1)
    {
	for(int i=(index/2); i>0; i--)
        {
	    dest[i*2]     = dest[i];
            dest[(i*2)+1] = dest[i];
        }
    }

    // return the number of samples in buffer
    return (index / channels);
}

/*
   Parse the the file to get metadata
 */

int SoundSourceOggVorbis::ParseHeader( TrackInfoObject * Track )
{
    QString filename = Track->getLocation();
    QByteArray qBAFilename = filename.toUtf8();
    vorbis_comment * comment;
    OggVorbis_File vf;

    FILE * vorbisfile = fopen(qBAFilename.data(), "r");
    if (!vorbisfile) {
        qDebug() << "oggvorbis: file cannot be opened.\n";
        return ERR;
    }

    // Apparently this is needed to make windows happy:
    #ifdef __WIN__
    _setmode( _fileno( vorbisfile ), _O_BINARY );
    #endif

    if (ov_open(vorbisfile, &vf, NULL, 0) < 0) {
        qDebug() << "oggvorbis: Input does not appear to be an Ogg bitstream.\n";
        return ERR;
    }

    comment = ov_comment(&vf, -1);

    if (QString(vorbis_comment_query(comment, "title", 0)).length()!=0)
        Track->setTitle(vorbis_comment_query(comment, "title", 0));
    if (QString(vorbis_comment_query(comment, "artist", 0)).length()!=0)
        Track->setArtist(vorbis_comment_query(comment, "artist", 0));
    if (QString(vorbis_comment_query(comment, "TBPM", 0)).length()!=0) {
        float bpm = str2bpm(vorbis_comment_query(comment, "TBPM", 0));
        if(bpm > 0) {
            Track->setBpm(bpm);
            Track->setBpmConfirm(true);
        }
    }
    Track->setHeaderParsed(true);

    Track->setType("ogg");
    Track->setDuration((int)ov_time_total(&vf, -1));
    Track->setBitrate(ov_bitrate(&vf, -1)/1000);

    vorbis_info * vi=ov_info(&vf,-1);
    Track->setSampleRate(vi->rate);
    Track->setChannels(vi->channels);

    ov_clear(&vf);
    return OK;
}

/*
   Return the length of the file in samples.
 */

inline long unsigned SoundSourceOggVorbis::length()
{
    return filelength;
}
