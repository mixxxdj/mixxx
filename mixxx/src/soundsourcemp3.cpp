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
    mad_stream_options(&Stream, MAD_OPTION_IGNORECRC);
    mad_stream_buffer(&Stream, (unsigned char *) inputbuf, inputbuf_len);

    /*
      Decode all the headers, and fill in stats:
    */
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

        // Add frame to list of frames
        MadSeekFrameType *p = new MadSeekFrameType;
        p->m_pStreamPos = (unsigned char*)Stream.this_frame;
        p->pos = length();
        m_qSeekList.append(p);

        currentframe++;
        mad_timer_add (&filelength, Header.duration);
        bitrate += Header.bitrate;
        SRATE = Header.samplerate;
        m_iChannels = MAD_NCHANNELS(&Header);

    }
    //qDebug("channels %i",m_iChannels);

    // Find average frame size
    m_iAvgFrameSize = length()/currentframe;

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

    Frame = new mad_frame;

    m_qSeekList.setAutoDelete(true);

    // Re-init buffer:
    seek(0);
}

SoundSourceMp3::~SoundSourceMp3()
{
    mad_stream_finish(&Stream);
    mad_frame_finish(Frame);
    mad_synth_finish(&Synth);
    delete [] inputbuf;

    m_qSeekList.clear();
}

long SoundSourceMp3::seek(long filepos)
{
    //qDebug("SEEK %i",filepos);

    MadSeekFrameType *cur;

    if (filepos==0)
    {
        // Seek to beginning of file

        // Re-init buffer:
        mad_stream_finish(&Stream);
        mad_stream_init(&Stream);
        mad_stream_options(&Stream, MAD_OPTION_IGNORECRC);
        mad_stream_buffer(&Stream, (unsigned char *) inputbuf, inputbuf_len);
        mad_frame_init(Frame);
        mad_synth_init(&Synth);
        rest=-1;
        cur = m_qSeekList.at(0);
    }
    else
    {
        //qDebug("seek precise");
        // Perform precise seek accomplished by using a frame in the seek list

        // Find the frame to seek to in the list
        /*
        MadSeekFrameType *cur = m_qSeekList.last();
        int k=0;
        while (cur!=0 && cur->pos>filepos)
        {
            cur = m_qSeekList.prev();
            ++k;
        }
        */

        int framePos = findFrame(filepos);

        //qDebug("list length %i, list pos %i",m_qSeekList.count(), k);

        if (framePos==0 || framePos>filepos || m_qSeekList.at()<5)
        {
            //qDebug("Problem finding good seek frame (wanted %i, got %i), starting from 0",filepos,framePos);

            // Re-init buffer:
            mad_stream_finish(&Stream);
            mad_stream_init(&Stream);
            mad_stream_options(&Stream, MAD_OPTION_IGNORECRC);
            mad_stream_buffer(&Stream, (unsigned char *) inputbuf, inputbuf_len);
            mad_frame_init(Frame);
            mad_synth_init(&Synth);
            rest = -1;
            cur = m_qSeekList.first();
        }
        else
        {
//            qDebug("frame pos %i",cur->pos);

            // Start four frame before wanted frame to get in sync...
            m_qSeekList.prev();
            m_qSeekList.prev();
            m_qSeekList.prev();
            cur = m_qSeekList.prev();

            // Start from the new frame
            mad_stream_finish(&Stream);
            mad_stream_init(&Stream);
            mad_stream_options(&Stream, MAD_OPTION_IGNORECRC);
	    //qDebug("mp3 restore %p",cur->m_pStreamPos);
            mad_stream_buffer(&Stream, (const unsigned char*)cur->m_pStreamPos, inputbuf_len-(long int)(cur->m_pStreamPos-(unsigned char*)inputbuf));
            mad_synth_mute(&Synth);
            mad_frame_mute(Frame);

            // Decode the three frames before
            mad_frame_decode(Frame,&Stream);
            mad_frame_decode(Frame,&Stream);
            mad_frame_decode(Frame,&Stream);
            if(mad_frame_decode(Frame,&Stream)) qDebug("MP3 decode warning");
            mad_synth_frame(&Synth, Frame);

            // Set current position
            rest = -1;
            m_qSeekList.next();
            m_qSeekList.next();
            m_qSeekList.next();
            cur = m_qSeekList.next();
        }

        // Synthesize the the samples from the frame which should be discard to reach the requested position
        SAMPLE *temp = new SAMPLE[READCHUNKSIZE];
        //int r = read(filepos-cur->pos, temp);
        //qDebug("try read %i, got %i...frame pos %i, filepos %i",filepos-cur->pos,r,cur->pos,filepos);
        //qDebug("ok");
        delete [] temp;
    }
/*
    else
    {
        qDebug("seek unprecise");
        // Perform seek which is can not be done precise because no frames is in the seek list

        int newpos = (int)(inputbuf_len * ((float)filepos/(float)length()));
//        qDebug("Seek to %d %d %d", filepos, inputbuf_len, newpos);

        // Go to an approximate position:
        mad_stream_buffer(&Stream, (unsigned char *) (inputbuf+newpos), inputbuf_len-newpos);
        mad_synth_mute(&Synth);
        mad_frame_mute(Frame);

        // Decode a few (possible wrong) buffers:
        int no = 0;
        int succesfull = 0;
        while ((no<10) && (succesfull<2))
        {
            if (!mad_frame_decode(Frame, &Stream))
            succesfull ++;
            no ++;
        }

        // Discard the first synth:
        mad_synth_frame(&Synth, Frame);

        // Remaining samples in buffer are useless
        rest = -1;

        // Reset seek frame list
        m_qSeekList.clear();
        MadSeekFrameType *p = new MadSeekFrameType;
        p->m_pStreamPos = (unsigned char*)Stream.this_frame;
        p->pos = filepos;
        m_qSeekList.append(p);
        m_iSeekListMinPos = filepos;
        m_iSeekListMaxPos = filepos;
        m_iCurFramePos = filepos;
    }
*/

    // Unfortunately we don't know the exact fileposition. The returned position is thus an
    // approximation only:
    return filepos;

}

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
//    qDebug("frame list %i",m_qSeekList.count());

    SAMPLE *destination = (SAMPLE *)_destination;
    unsigned Total_samples_decoded = 0;

    // If samples are left from previous read, then copy them to start of destination
    if (rest > 0)
    {
        for (int i=rest; i<Synth.pcm.length; i++)
        {
            // Left channel
            *(destination++) = madScale(Synth.pcm.samples[0][i]);

            /* Right channel. If the decoded stream is monophonic then
             * the right output channel is the same as the left one. */
            if (m_iChannels>1)
                *(destination++) = madScale(Synth.pcm.samples[1][i]);
            else
                *(destination++) = madScale(Synth.pcm.samples[0][i]);
        }
        Total_samples_decoded += 2*(Synth.pcm.length-rest);
    }

    //qDebug("Decoding");
    int no = 0;
    int frames = 0;
    while (Total_samples_decoded < samples_wanted)
    {
        //qDebug("no %i",Total_samples_decoded);
        if(mad_frame_decode(Frame,&Stream))
        {
            if(MAD_RECOVERABLE(Stream.error))
            {
                //qDebug("MAD: Recoverable frame level ERR (%s)",mad_stream_errorstr(&Stream));
                continue;
            } else if(Stream.error==MAD_ERROR_BUFLEN) {
                //qDebug("MAD: buflen ERR");
                break;
            } else {
                //qDebug("MAD: Unrecoverable frame level ERR (%s).",mad_stream_errorstr(&Stream));
                break;
            }
        }

        ++frames;

        /* Once decoded the frame is synthesized to PCM samples. No ERRs
         * are reported by mad_synth_frame();
         */
        mad_synth_frame(&Synth,Frame);

        // Number of channels in frame
        //ch = MAD_NCHANNELS(&Frame->header);

        /* Synthesized samples must be converted from mad's fixed
         * point number to the consumer format (16 bit). Integer samples
         * are temporarily stored in a buffer that is flushed when
         * full.
         */


        //qDebug("synthlen %i, remain %i",Synth.pcm.length,(samples_wanted-Total_samples_decoded));
        no = min(Synth.pcm.length,(samples_wanted-Total_samples_decoded)/2);
        for (int i=0; i<no; i++)
        {
            // Left channel
            *(destination++) = madScale(Synth.pcm.samples[0][i]);

            /* Right channel. If the decoded stream is monophonic then
             * the right output channel is the same as the left one. */
            if (m_iChannels==2)
                *(destination++) = madScale(Synth.pcm.samples[1][i]);
            else
                *(destination++) = madScale(Synth.pcm.samples[0][i]);
        }
        Total_samples_decoded += 2*no;

        //qDebug("decoded: %i, wanted: %i",Total_samples_decoded,samples_wanted);
    }

    // If samples are still left in buffer, set rest to the index of the unused samples
    if (Synth.pcm.length > no)
        rest = no;
    else
        rest = -1;

    //qDebug("decoded %i samples in %i frames, rest: %i, chan %i", Total_samples_decoded, frames, rest, m_iChannels);
    return Total_samples_decoded;
}

int SoundSourceMp3::ParseHeader(TrackInfoObject *Track)
{
    QString location = Track->getLocation();

    Track->setType("mp3");

    id3_file *fh = id3_file_open(location.latin1(), ID3_FILE_MODE_READONLY);
    if (fh!=0)
    {
        id3_tag *tag = id3_file_tag(fh);
        if (tag!=0)
        {
            QString s;
            getField(tag,"TIT2",&s);
            if (s.length()>2)
                Track->setTitle(s);
            s="";
            getField(tag,"TPE1",&s);
            if (s.length()>2)
                Track->setArtist(s);

            /*
            // On some tracks this segfaults. TLEN is very seldom used anyway...
            QString dur;
            getField(tag,"TLEN",&dur);
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
        qDebug("MAD: Open of %s failed.", location.latin1());
        return ERR;
    }
    char *inputbuf = new char[READLENGTH];
    unsigned int tmp = file.readBlock(inputbuf, READLENGTH);
    if (tmp != READLENGTH) {
        qDebug("MAD: ERR reading mp3-file: %s\nRead only %d bytes, but wanted %d bytes.",location.latin1() ,tmp,READLENGTH);
        return ERR;
    }
    mad_stream Stream;
    mad_header Header;
    mad_stream_init(&Stream);
    mad_stream_options(&Stream, MAD_OPTION_IGNORECRC);
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
        Track->setDuration(dur.seconds);
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
            mad_timer_multiply(&dur, Track->getLength()/((Stream.this_frame-Stream.buffer)/frames));
            Track->setDuration(dur.seconds);
            Track->setBitrate(Header.bitrate/1000);
        }
//        else
//            qDebug("MAD: Count frames to get file duration!");
    }

    mad_stream_finish(&Stream);
    delete [] inputbuf;
    file.close();
    return OK;
}

void SoundSourceMp3::getField(id3_tag *tag, const char *frameid, QString *str)
{
    id3_frame *frame = id3_tag_findframe(tag, frameid, 0);
    if (frame)
    {
        // Unicode handling
        if (id3_field_getnstrings(&frame->fields[1])>0)
        {
            id3_utf16_t *framestr = id3_ucs4_utf16duplicate(id3_field_getstrings(&frame->fields[1], 0));
            int strlen = 0; while (framestr[strlen]!=0) strlen++;
            if (strlen>0)
                str->setUnicodeCodes((ushort *)framestr,strlen);
            free(framestr);
        }
    }
}

int SoundSourceMp3::findFrame(int pos)
{
    // Guess position of frame in m_qSeekList based on average frame size
    MadSeekFrameType *temp = m_qSeekList.at(min(m_qSeekList.count()-1, (unsigned int)(pos/m_iAvgFrameSize)));

/*
    if (temp!=0)
        qDebug("find %i, got %i",pos, temp->pos);
    else
        qDebug("find %i, tried idx %i, total %i",pos, min(m_qSeekList.count()-1, pos/m_iAvgFrameSize),m_qSeekList.count());
*/

    // Ensure that the list element is not at a greater position than pos
    while (temp!=0 && temp->pos>pos)
    {
        temp = m_qSeekList.prev();
//        if (temp!=0) qDebug("backing %i, got %i",pos,temp->pos);
    }

    // Ensure that the following position is also not smaller than pos
    if (temp!=0)
    {
        temp = m_qSeekList.current();
        while (temp!=0 && temp->pos<pos)
        {
            temp = m_qSeekList.next();
//            if (temp!=0) qDebug("fwd'ing %i, got %i",pos,temp->pos);
        }

        if (temp==0)
            temp = m_qSeekList.last();
        else
            temp = m_qSeekList.prev();
    }

    if (temp>0)
    {
//        qDebug("ended at %i, got %i",pos,temp->pos);
        return temp->pos;
    }
    else
    {
//        qDebug("ended at 0");
        return 0;
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

