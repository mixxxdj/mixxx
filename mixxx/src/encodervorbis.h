/***************************************************************************
                     encodervorbis.h  -  vorbis encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
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

#include <QObject>
#include "defs.h"
#include "configobject.h"
#include "encoder.h"

#include <vorbis/vorbisenc.h> // this also includes vorbis/codec.h

class EngineAbstractRecord;
class TrackInfoObject;

class EncoderVorbis : public Encoder {
    Q_OBJECT

public:
    EncoderVorbis(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine=0);
    ~EncoderVorbis();
    int initEncoder(int bitrate);
	void encodeBuffer(const CSAMPLE *samples, const int size);
	void updateMetaData(char* artist, char* title);

private slots:
    

private:
    int getSerial();
    void flushStream();
    void initStream();
    void sendPackages();
    bool metaDataHasChanged();

    ConfigObject<ConfigValue> *m_pConfig; /* provides ConfigKey access */
    ogg_stream_state oggs;    /* take physical pages, weld into logical stream
                                 of packets */
    ogg_page oggpage;         /* Ogg bitstream page: contains Vorbis packets */
    ogg_packet oggpacket;     /* raw packet of data */
    vorbis_block vblock;      /* local working space for packet-to-PCM */
    vorbis_dsp_state vdsp;    /* central working space for packet-to-PCM */
    vorbis_info vinfo;        /* stores all static vorbis bitstream settings */
    vorbis_comment vcomment;  /* stores all user comments */
	bool header_write;

    EngineAbstractRecord *pEngine;
    TrackInfoObject *m_pMetaData;
    char *metaDataTitle;
    char *metaDataArtist;
};

#endif
