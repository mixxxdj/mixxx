/***************************************************************************
                          encodervorbis.h  -  description
                             -------------------
    copyright            : (C) 2007 by Xiph.org
                           (C) 2007 by Wesley Stessens
    email                : wesley at ubuntu dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
Okay, so this is the vorbis encoder class...
It's a real mess right now.

When I get around to cleaning things up,
I'll probably make an Encoder base class,
so we can easily add in an EncoderLame (or something) too.

A lot of stuff has been stolen from:
http://svn.xiph.org/trunk/vorbis/examples/encoder_example.c
*/

#include "encodervorbis.h"

#include <stdlib.h> // needed for random num gen
#include <time.h> // needed for random num gen
#include <string.h> // needed for memcpy
#include <QDebug>
#include <stdio.h> // currently used for writing to stdout

// Constructor
EncoderVorbis::EncoderVorbis()
{
}

// Destructor
EncoderVorbis::~EncoderVorbis()
{
  ogg_stream_clear(&oggs);
  vorbis_block_clear(&vblock);
  vorbis_dsp_clear(&vdsp);
  vorbis_comment_clear(&vcomment);
  vorbis_info_clear(&vinfo);
}

/*
  Get new random serial number
  -> returns random number
*/
int EncoderVorbis::getSerial()
{
    static int prevSerial = 0;
    int serial = 0;
    while (prevSerial == serial)
        serial = rand();
    prevSerial = serial;
    return serial;
}

/*
This stuff is probably real ugly and buggy.
This is the first time I work more directly with memory.
It's fun, but it's not all too easy for me.
*/
int EncoderVorbis::encodeBuffer(const CSAMPLE *samples, const int size)
{
    float **buffer;
    int i;
    buffer = vorbis_analysis_buffer(&vdsp, size);
/*
No deinterleaving, just simple memcpy
*/

/*    for(i=0; i < vinfo.channels; i++)
    {
        memcpy(buffer[i], &samples[i], size*sizeof(CSAMPLE));
    }*/

/*
I tried to deinterleave some stuff here..
What I do here is not right..
I should think about it better..
Also: hardcoded 2 channels right now, just for testing
*/
/*int j;
        for( j = 0 ; j < size ; j++ )
        {
            buffer[0][j]= samples[j*2+1]; //3; 5; 7; 9; 11
            buffer[1][j]= samples[j*2+0]; //2; 4; 6; 8; 10
//            buffer[0][j]= samples[j];
//            buffer[1][j]= buffer[0][j];
        }*/
    for (i = 0; i < size/2; ++i)
    {
        buffer[0][i] = samples[i*2]/32768.f; // 0; 2; 4
        buffer[1][i] = samples[i*2+1]/32768.f; // 1; 3; 5
    }

    vorbis_analysis_wrote(&vdsp, i);

    while (vorbis_analysis_blockout(&vdsp, &vblock) == 1) {
        vorbis_analysis(&vblock, 0);
        vorbis_bitrate_addblock(&vblock);
        while (vorbis_bitrate_flushpacket(&vdsp, &oggpacket)) {
            // weld packet into bitstream
            ogg_stream_packetin(&oggs, &oggpacket);
            // write out pages
            int eos = 0;
            while (!eos) {
                int result = ogg_stream_pageout(&oggs, &oggpage);
                if (result == 0) break;
                qDebug() << "emit pageReady()";
//                emit pageReady(oggpage.header, oggpage.body, oggpage.header_len, oggpage.body_len);
//                memcpy(h, oggpage.header, oggpage.header_len);
//                memcpy(b, oggpage.body, oggpage.body_len);
                fwrite(oggpage.header,1,oggpage.header_len,stdout);
                fwrite(oggpage.body,1,oggpage.body_len,stdout);
                if (ogg_page_eos(&oggpage)) eos = 1;
            }
        }
    }
//    *bodySize = oggpage.body_len;
    return 1337;
}

/*
  Initialize the encoder
  -> returns -1 on error
  -> returns 0 on success
*/
int EncoderVorbis::init()
{
    int ret, result;
    vorbis_info_init(&vinfo);

    // initialize VBR quality based mode
    ret = vorbis_encode_init_vbr(&vinfo, 2, 48000, 0.4);

    if (ret == 0) {
        // add comment
        vorbis_comment_init(&vcomment);
        vorbis_comment_add_tag(&vcomment, "ENCODER", "mixxx");
        vorbis_comment_add_tag(&vcomment, "ARTIST", "testArtist");
        vorbis_comment_add_tag(&vcomment, "TITLE", "testSong");

        // set up analysis state and auxiliary encoding storage
        vorbis_analysis_init(&vdsp, &vinfo);
        vorbis_block_init(&vdsp, &vblock);

        // set up packet-to-stream encoder; attach a random serial number
        srand(time(0));
        ogg_stream_init(&oggs, getSerial());

        // set up the vorbis headers
        ogg_packet headerInit;
        ogg_packet headerComment;
        ogg_packet headerCode;
        vorbis_analysis_headerout(&vdsp, &vcomment, &headerInit, &headerComment, &headerCode);
        ogg_stream_packetin(&oggs, &headerInit);
        ogg_stream_packetin(&oggs, &headerComment);
        ogg_stream_packetin(&oggs, &headerCode);

        // start audio data on new page
        while (1) {
            result = ogg_stream_flush(&oggs, &oggpage);
            if (result==0) break;
                qDebug() << "emit pageReady() main_header";
//                emit pageReady(oggpage.header, oggpage.body, oggpage.header_len, oggpage.body_len);
/*
Instead of writing to stdout I should probably save the header+body and
paste it in front of the first result of the call to EncodeBuffer
*/
                fwrite(oggpage.header,1,oggpage.header_len,stdout);
                fwrite(oggpage.body,1,oggpage.body_len,stdout);
        }
    } else {
        ret = -1;
    };
    return ret;
}
