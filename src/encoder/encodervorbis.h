/***************************************************************************
                     encodervorbis.h  -  vorbis encoder for mixxx
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

#ifndef ENCODERVORBIS_H
#define ENCODERVORBIS_H

// this also includes vorbis/codec.h
#include <vorbis/vorbisenc.h>

#include "defs.h"
#include "encoder/encoder.h"
#include "trackinfoobject.h"

class EncoderCallback;

class EncoderVorbis : public Encoder {
  public:
    EncoderVorbis(EncoderCallback* pCallback=NULL);
    virtual ~EncoderVorbis();

    int initEncoder(int bitrate, int samplerate);
    void encodeBuffer(const CSAMPLE *samples, const int size);
    void updateMetaData(char* artist, char* title, char* album);
    void flush();

  private:
    int getSerial();
    void initStream();
    bool metaDataHasChanged();
    //Call this method in conjunction with shoutcast streaming
    void writePage();

    bool m_bStreamInitialized;
    ogg_stream_state m_oggs;    /* take physical pages, weld into logical stream
                                 of packets */
    ogg_page m_oggpage;         /* Ogg bitstream page: contains Vorbis packets */
    ogg_packet m_oggpacket;     /* raw packet of data */
    vorbis_block m_vblock;      /* local working space for packet-to-PCM */
    vorbis_dsp_state m_vdsp;    /* central working space for packet-to-PCM */
    vorbis_info m_vinfo;        /* stores all static vorbis bitstream settings */
    vorbis_comment m_vcomment;  /* stores all user comments */
    bool m_header_write;

    EncoderCallback* m_pCallback;
    TrackPointer m_pMetaData;
    char* m_metaDataTitle;
    char* m_metaDataArtist;
    char* m_metaDataAlbum;
    QFile m_file;
};

#endif
