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


#include "soundsourceoggvorbis.h"

/*
  Class for reading Ogg Vorbis 
*/

SoundSourceOggVorbis::SoundSourceOggVorbis(const char* filename)
{
    vorbisfile =  fopen(filename, "r");

    if(ov_open(vorbisfile, &vf, NULL, 0) < 0) {
       qDebug("oggvorbis: Input does not appear to be an Ogg bitstream.\n");
       filelength = 0;
       return;
    } else
    {

      // extract metadata
      vorbis_info *vi=ov_info(&vf,-1);

      channels = vi->channels;
      SRATE = vi->rate;

      if(channels > 2){
        qDebug("oggvorbis: No support for more than 2 channels!\n");
        return;
      }
        
      filelength = (unsigned long) ov_pcm_total(&vf, -1) * 2;

      type = "ogg vorbis file.";
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
    }else{
        qDebug("ogg vorbis: Seek error.");
        return 0;
    }
}


/*
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/

unsigned SoundSourceOggVorbis::read(unsigned long size, const SAMPLE* destination)
{
    dest = (SAMPLE*) destination;
    index = 0;
    needed = size * channels;

    // loop until requested number of samples has been retrieved
    while (needed > 0){
        // read samples into buffer
        ret = ov_read(&vf,(char*) dest+index,needed, 0, 2, 1, &current_section);
        // if eof we fill the rest with zero
        if (ret == 0) {
            while (needed > 0){
                dest[index] = 0;
                index++;
                needed--;
              }   
        }
        index += ret;
        needed -= ret;
    }

    // convert into stereo if file is mono
    if (channels == 1){
        for(int i=index;i>0;i--){
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

void SoundSourceOggVorbis::ParseHeader( TrackInfoObject *Track )
{
    QString filename = Track->m_sFilepath+'/'+Track->m_sFilename;
    vorbis_comment *comment;
    OggVorbis_File vf;
       
    FILE* vorbisfile = fopen(filename, "r");
            
    if (ov_open(vorbisfile, &vf, NULL, 0) < 0) {
        qDebug("oggvorbis: Input does not appear to be an Ogg bitstream.\n");
        return;
    }

    comment = ov_comment(&vf, -1);

    Track->m_sTitle = vorbis_comment_query(comment, "title", 0);
    Track->m_sArtist = vorbis_comment_query(comment, "artist", 0);
    Track->m_sType = "ogg";
    Track->m_iDuration = (long) ov_time_total(&vf, -1);
    Track->m_sBitrate.setNum(ov_bitrate(&vf, -1)/1000);

    ov_clear(&vf);
}

/*
  Return the length of the file in samples.
*/

inline long unsigned SoundSourceOggVorbis::length()
{
    return filelength;
}
