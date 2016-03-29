/****************************************************************************
                   encodervorbis.cpp  -  vorbis encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) 1994 Tobias Rafreider (shoutcast and recording fixes)
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

#include <stdlib.h> // needed for random num gen
#include <time.h> // needed for random num gen
#include <QtDebug>

#include "encoder/encodervorbis.h"
#include "encoder/encodercallback.h"
#include "errordialoghandler.h"

EncoderVorbis::EncoderVorbis(EncoderCallback* pCallback)
        : m_bStreamInitialized(false),
          m_header_write(false),
          m_pCallback(pCallback),
          m_metaDataTitle(NULL),
          m_metaDataArtist(NULL),
          m_metaDataAlbum(NULL){
    m_vdsp.pcm_returned = 0;
    m_vdsp.preextrapolate = 0;
    m_vdsp.eofflag = 0;
    m_vdsp.lW = 0;
    m_vdsp.W = 0;
    m_vdsp.nW = 0;
    m_vdsp.centerW = 0;
    m_vdsp.granulepos = 0;
    m_vdsp.sequence = 0;
    m_vdsp.glue_bits = 0;
    m_vdsp.time_bits = 0;
    m_vdsp.floor_bits = 0;
    m_vdsp.res_bits = 0;
    m_vdsp.backend_state = NULL;

    m_vinfo.version = 0;
    m_vinfo.channels = 0;
    m_vinfo.rate = 0;
    m_vinfo.bitrate_upper = 0;
    m_vinfo.bitrate_nominal = 0;
    m_vinfo.bitrate_lower = 0;
    m_vinfo.bitrate_window = 0;
    m_vinfo.codec_setup = NULL;

    m_vcomment.user_comments = NULL;
    m_vcomment.comment_lengths = NULL;
    m_vcomment.comments = 0;
    m_vcomment.vendor = NULL;
}

EncoderVorbis::~EncoderVorbis() {
    if (m_bStreamInitialized) {
        ogg_stream_clear(&m_oggs);
        vorbis_block_clear(&m_vblock);
        vorbis_dsp_clear(&m_vdsp);
        vorbis_comment_clear(&m_vcomment);
        vorbis_info_clear(&m_vinfo);
    }
}

// call sendPackages() or write() after 'flush()' as outlined in engineshoutcast.cpp
void EncoderVorbis::flush() {
    vorbis_analysis_wrote(&m_vdsp, 0);
    writePage();
}

/*
  Get new random serial number
  -> returns random number
*/
int EncoderVorbis::getSerial()
{
    static int prevSerial = 0;
    int serial = rand();
    while (prevSerial == serial)
        serial = rand();
    prevSerial = serial;
    qDebug() << "RETURNING SERIAL " << serial;
    return serial;
}

void EncoderVorbis::writePage() {

    /*
     * Vorbis streams begin with three headers; the initial header (with
     * most of the codec setup parameters) which is mandated by the Ogg
     * bitstream spec.  The second header holds any comment fields.  The
     * third header holds the bitstream codebook.  We merely need to
     * make the headers, then pass them to libvorbis one at a time;
     * libvorbis handles the additional Ogg bitstream constraints
         */


    // Write header only once after stream has been initialized
    int result;
    if (m_header_write) {
        while (true) {
            result = ogg_stream_flush(&m_oggs, &m_oggpage);
            if (result == 0)
                break;
            m_pCallback->write(m_oggpage.header, m_oggpage.body, m_oggpage.header_len, m_oggpage.body_len);
        }
        m_header_write = false;
    }

    while (vorbis_analysis_blockout(&m_vdsp, &m_vblock) == 1) {
        vorbis_analysis(&m_vblock, 0);
        vorbis_bitrate_addblock(&m_vblock);
        while (vorbis_bitrate_flushpacket(&m_vdsp, &m_oggpacket)) {
            // weld packet into bitstream
            ogg_stream_packetin(&m_oggs, &m_oggpacket);
            // write out pages
            bool eos = false;
            while (!eos) {
                int result = ogg_stream_pageout(&m_oggs, &m_oggpage);
                if (result == 0)
                    break;
                m_pCallback->write(m_oggpage.header, m_oggpage.body,
                                   m_oggpage.header_len, m_oggpage.body_len);
                if (ogg_page_eos(&m_oggpage))
                    eos = true;
            }
        }
    }
}

void EncoderVorbis::encodeBuffer(const CSAMPLE *samples, const int size) {
    float **buffer = vorbis_analysis_buffer(&m_vdsp, size);

    // Deinterleave samples. We use normalized floats in the engine [-1.0, 1.0]
    // and libvorbis expects samples in the range [-1.0, 1.0] so no conversion
    // is required.
    for (int i = 0; i < size/2; ++i) {
        buffer[0][i] = samples[i*2];
        buffer[1][i] = samples[i*2+1];
    }
    /** encodes audio **/
    vorbis_analysis_wrote(&m_vdsp, size/2);
    /** writes the OGG page and sends it to file or stream **/
    writePage();
}

/* Originally called from engineshoutcast.cpp to update metadata information
 * when streaming, however, this causes pops
 *
 * Currently this method is used before init() once to save artist, title and album
*/
void EncoderVorbis::updateMetaData(char* artist, char* title, char* album) {
    m_metaDataTitle = title;
    m_metaDataArtist = artist;
    m_metaDataAlbum = album;
}

void EncoderVorbis::initStream() {
    // set up analysis state and auxiliary encoding storage
    vorbis_analysis_init(&m_vdsp, &m_vinfo);
    vorbis_block_init(&m_vdsp, &m_vblock);

    // set up packet-to-stream encoder; attach a random serial number
    srand(time(0));
    ogg_stream_init(&m_oggs, getSerial());

    // add comment
    vorbis_comment_init(&m_vcomment);
    vorbis_comment_add_tag(&m_vcomment, "ENCODER", "mixxx/libvorbis");
    if (m_metaDataArtist != NULL) {
        vorbis_comment_add_tag(&m_vcomment, "ARTIST", m_metaDataArtist);
    }
    if (m_metaDataTitle != NULL) {
        vorbis_comment_add_tag(&m_vcomment, "TITLE", m_metaDataTitle);
    }
    if (m_metaDataAlbum != NULL) {
        vorbis_comment_add_tag(&m_vcomment, "ALBUM", m_metaDataAlbum);
    }

    // set up the vorbis headers
    ogg_packet headerInit;
    ogg_packet headerComment;
    ogg_packet headerCode;
    vorbis_analysis_headerout(&m_vdsp, &m_vcomment, &headerInit, &headerComment, &headerCode);
    ogg_stream_packetin(&m_oggs, &headerInit);
    ogg_stream_packetin(&m_oggs, &headerComment);
    ogg_stream_packetin(&m_oggs, &headerCode);

    // The encoder is now inialized. The encode method will start streaming by
    // sending the header first.
    m_header_write = true;
    m_bStreamInitialized = true;
}

int EncoderVorbis::initEncoder(int bitrate, int samplerate) {
    vorbis_info_init(&m_vinfo);

    // initialize VBR quality based mode
    int ret = vorbis_encode_init(&m_vinfo, 2, samplerate, -1, bitrate*1000, -1);

    if (ret == 0) {
        initStream();
    } else {
        ret = -1;
    };
    return ret;
}
