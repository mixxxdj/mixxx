/***************************************************************************
                          soundsourcemp3.cpp  -  description
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

#include "soundsourcemp3.h"
#include "trackinfoobject.h"
extern "C" {
#include <dxhead.h>
}

SoundSourceMp3::SoundSourceMp3(QString qFilename) : SoundSource(qFilename)
{
    QFile file( qFilename.latin1() );
    if (!file.open(IO_ReadOnly))
        qFatal("MAD: Open of %s failed.", qFilename.latin1());

    // Read the whole file into inputbuf:
    inputbuf_len = file.size();
    inputbuf = new char[inputbuf_len];
    unsigned int tmp = file.readBlock(inputbuf, inputbuf_len);
    if (tmp != inputbuf_len)
        qFatal("MAD: ERR reading mp3-file: %s\nRead only %d bytes, but wanted %d bytes.",qFilename.latin1() ,tmp,inputbuf_len);

    // Transfer it to the mad stream-buffer:
    mad_stream_init(&Stream);
    mad_stream_buffer(&Stream, (unsigned char *) inputbuf, inputbuf_len);

    /*
      Decode all the headers, and fill in stats:
    */
//    int len = 0;
    mad_header Header;
    filelength = mad_timer_zero;
    bitrate = 0;
    currentframe = 0;
    pos = mad_timer_zero;

    while ((Stream.bufend - Stream.this_frame) > 0)
    {
        if (mad_header_decode (&Header, &Stream) == -1)
        {
            if (!MAD_RECOVERABLE (Stream.error))
                break;
           /* if (Stream.ERR == MAD_ERR_LOSTSYNC)
            {
                // ignore LOSTSYNC due to ID3 tags 
                int tagsize = id3_tag_query (Stream.this_frame,
                                Stream.bufend - Stream.this_frame);
                if (tagsize > 0)
                {
                    mad_stream_skip (&Stream, tagsize);
                    continue;
                }
            }*/

            //qDebug("MAD: ERR decoding header %d: %s (len=%d)", currentframe, mad_stream_ERRstr(&Stream), len);
            continue;
        }
        currentframe++;
        mad_timer_add (&filelength, Header.duration);
        bitrate += Header.bitrate;
        SRATE = Header.samplerate;
    }

    mad_header_finish (&Header);
    if (currentframe==0)
        bitrate = 0;
    else
        bitrate = bitrate/currentframe;
    framecount = currentframe;
    currentframe = 0;

    /*
    qDebug("length  = %ld sec." , filelength.seconds);
    qDebug("frames  = %d" , framecount);
    qDebug("bitrate = %d" , bitrate/1000);
    qDebug("Size    = %d", length());
    */

    // Re-init buffer:
    mad_stream_finish(&Stream);
    mad_stream_init(&Stream);
    mad_stream_buffer(&Stream, (unsigned char *) inputbuf, inputbuf_len);
    mad_frame_init(&Frame);
    mad_synth_init(&Synth);

    // Set the type field:
    type = "mp3 file.";

    rest = -1;
}

SoundSourceMp3::~SoundSourceMp3()
{
    mad_stream_finish(&Stream);
    mad_frame_finish(&Frame);
    mad_synth_finish(&Synth);
    delete [] inputbuf;
}

long SoundSourceMp3::seek(long filepos)
{
    int newpos = (int)(inputbuf_len * ((float)filepos/(float)length()));
    //qDebug("Seek to %d %d %d", filepos, inputbuf_len, newpos);

    // Go to an approximate position:
    mad_stream_buffer(&Stream, (unsigned char *) (inputbuf+newpos), inputbuf_len-newpos);
    mad_synth_mute(&Synth);
    mad_frame_mute(&Frame);

    // Decode a few (possible wrong) buffers:
    int no = 0;
    int succesfull = 0;
    while ((no<10) && (succesfull<2)) {
        if (!mad_frame_decode(&Frame, &Stream))
            succesfull ++;
        no ++;
    }

    // Discard the first synth:
    mad_synth_frame(&Synth, &Frame);

    // Remaining samples in buffer are useless
    rest = -1;

    // Unfortunately we don't know the exact fileposition. The returned position is thus an
    // approximation only:
    return filepos;
}

/*    FILE *file;
  Return the length of the file in samples.
*/
inline long unsigned SoundSourceMp3::length()
{
    return (long unsigned) 2*mad_timer_count(filelength, MAD_UNITS_44100_HZ);
}

/*	
  read <size> samples into <destination>, and return the number of
  samples actually read.
*/
unsigned SoundSourceMp3::read(unsigned long samples_wanted, const SAMPLE* _destination)
{
    SAMPLE *destination = (SAMPLE *)_destination;
    unsigned Total_samples_decoded = 0;
    int frames = 0;

    // If samples are left from previous read, then copy them to start of destination
    if (rest > 0)
    {
        for (int i=rest; i<Synth.pcm.length; i++)
        {
            // Left channel
            *(destination++) = madScale(Synth.pcm.samples[0][i]);

            /* Right channel. If the decoded stream is monophonic then
             * the right output channel is the same as the left one. */
            if (MAD_NCHANNELS(&Frame.header)==2)
                *(destination++) = madScale(Synth.pcm.samples[1][i]);
            else
                *(destination++) = madScale(Synth.pcm.samples[0][i]);
        }
        Total_samples_decoded += 2*(Synth.pcm.length-rest);
    }
    
    //qDebug("Decoding");
    int no;
    while (Total_samples_decoded < samples_wanted)
    {
        if(mad_frame_decode(&Frame,&Stream))
        {
            if(MAD_RECOVERABLE(Stream.error))
            {
                //qDebug("MAD: Recoverable frame level ERR (%s)",
                //       mad_stream_ERRstr(&Stream));
                continue;
            } else if(Stream.error==MAD_ERROR_BUFLEN) {
                //qDebug("MAD: buflen ERR");
                break;
            } else {
                //qDebug("MAD: Unrecoverable frame level ERR (%s).",
                //mad_stream_ERRstr(&Stream));
                break;
            }
        }
        /* Once decoded the frame is synthesized to PCM samples. No ERRs
         * are reported by mad_synth_frame();
         */
        mad_synth_frame(&Synth,&Frame);
        frames ++;
        /* Synthesized samples must be converted from mad's fixed
         * point number to the consumer format (16 bit). Integer samples
         * are temporarily stored in a buffer that is flushed when
         * full.
         */
        no = min(Synth.pcm.length,(samples_wanted-Total_samples_decoded)/2);
        for (int i=0;i<no;i++)
        {
            // Left channel
            *(destination++) = madScale(Synth.pcm.samples[0][i]);

            /* Right channel. If the decoded stream is monophonic then
             * the right output channel is the same as the left one. */
            if (MAD_NCHANNELS(&Frame.header)==2)
                *(destination++) = madScale(Synth.pcm.samples[1][i]);
            else
                *(destination++) = madScale(Synth.pcm.samples[0][i]);
        }
        Total_samples_decoded += 2*no;

//        qDebug("decoded: %i, wanted: %i",Total_samples_decoded,samples_wanted);
    }

    // If samples are still left in buffer, set rest to the index of the unused samples
    if (Synth.pcm.length > no)
        rest = no;
    else
        rest = -1;
  
//    qDebug("decoded %i samples in %i frames, rest: %i.", Total_samples_decoded, frames, rest);
    return Total_samples_decoded;
}

int SoundSourceMp3::ParseHeader(TrackInfoObject *Track)
{
    QString location = Track->m_sFilepath+'/'+Track->m_sFilename;

    Track->m_sType = "mp3";

    id3_file *fh = id3_file_open(location.latin1(), ID3_FILE_MODE_READONLY);
    if (fh!=0)
    {
        id3_tag *tag = id3_file_tag(fh);
        if (tag!=0)
        {
            getField(tag,"TIT2",Track->m_sTitle);
            getField(tag,"TPE1",Track->m_sArtist);

            /*
            // On some tracks this segfaults. TLEN is very seldom used anyway...
            QString dur;
            getField(tag,"TLEN",dur);
            if (dur.length()>0)
                Track->m_iDuration = dur.toInt();
            */
        }
        id3_file_close(fh);
    } 
    else
        return ERR;

    // Get file length. This has to be done by one of these options:
    // 1) looking for the tag named TLEN (above),
    // 2) See if the first frame contains a Xing header to get frame count
    // 3) If file does not contain Xing header, find out if it is a variable frame size file
    //    by looking at the size of the first 10 frames. If constant size, estimate frame number
    //    from one frame size and file length in bytes
    // 4) Count all the frames (slooow)

    // Open file, initialize MAD and read beginnning of file

    // Number of bytes to read from file to determine duration
    const unsigned int READLENGTH = 5000;
    mad_timer_t dur = mad_timer_zero;
    QFile file(location.latin1());
    if (!file.open(IO_ReadOnly)) {
        qWarning("MAD: Open of %s failed.", location.latin1());
        return ERR;
    }
    char *inputbuf = new char[READLENGTH];
    unsigned int tmp = file.readBlock(inputbuf, READLENGTH);
    if (tmp != READLENGTH) {
        qWarning("MAD: ERR reading mp3-file: %s\nRead only %d bytes, but wanted %d bytes.",location.latin1() ,tmp,READLENGTH);
        return ERR;
    }
    mad_stream Stream;
    mad_header Header;
    mad_stream_init(&Stream);
    mad_stream_buffer(&Stream, (unsigned char *) inputbuf, READLENGTH);

    // Check for Xing header
    XHEADDATA *xing = new XHEADDATA;
    xing->toc = 0;
    bool foundxing = false;
    if (GetXingHeader(xing, (unsigned char *)Stream.this_frame)==1)
    {
        foundxing = true;

        if (mad_header_decode (&Header, &Stream) != -1)
        {
            dur = Header.duration;
            mad_timer_multiply(&dur,xing->frames);
        }
    }
    delete xing;

    if (foundxing)
    {
        Track->m_iDuration = dur.seconds;
    }
    else
    {
        // Check if file has constant bit rate by examining the rest of the buffer
        unsigned long bitrate;
        int i=0;
        bool constantbitrate = true;
        int frames = 0;
        while ((Stream.bufend - Stream.this_frame) > 0)
        {
            if (mad_header_decode (&Header, &Stream) == -1)
            {
                if (!MAD_RECOVERABLE (Stream.error))
                    break;
            }
            if (i==0)
            {
                bitrate = Header.bitrate;
                dur = Header.duration;
            }
            else if (bitrate != Header.bitrate)
                constantbitrate = false;

            frames++;
        }
        if (constantbitrate && frames>1)
        {
            mad_timer_multiply(&dur, Track->m_iLength/((Stream.this_frame-Stream.buffer)/frames));
            Track->m_iDuration = dur.seconds;
            Track->m_sBitrate.setNum(Header.bitrate/1000);
        }
        else
            qDebug("MAD: Count frames to get file duration!");
    }
    
    mad_stream_finish(&Stream);
    delete [] inputbuf;
    file.close();
    return OK;
}

void SoundSourceMp3::getField(id3_tag *tag, const char *frameid, QString str)
{
    id3_frame *frame = id3_tag_findframe(tag, frameid, 0);
    if (frame)
    {
        /*
        // Latin1 handling
        union id3_field const *field = &frame->fields[1];
        const char *s = id3_ucs4_latin1duplicate(id3_field_getstrings(field, 0));
        str = s;
        delete [] s;
        */
        
        // Unicode handling.
        id3_utf16_t *framestr = id3_ucs4_utf16duplicate(id3_field_getstrings(&frame->fields[1], 0));
        int strlen = 0; while (framestr[strlen]!=0) strlen++;
        if (strlen>0)
            str.setUnicodeCodes((ushort *)framestr,strlen);
        delete [] framestr;
    }
}

inline signed int SoundSourceMp3::madScale (mad_fixed_t sample)
{
    sample += (1L << (MAD_F_FRACBITS - 16));

    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    return sample >> (MAD_F_FRACBITS + 1 - 16);
}
