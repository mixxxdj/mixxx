/****************************************************************************
                   encodervorbis.cpp  -  vorbis encoder for mixxx
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

/*
Okay, so this is the vorbis encoder class...
It's a real mess right now.

When I get around to cleaning things up,
I'll probably make an Encoder base class,
so we can easily add in an EncoderLame (or something) too.

A lot of stuff has been stolen from:
http://svn.xiph.org/trunk/vorbis/examples/encoder_example.c
*/

// TODO: MORE ERROR CHECKING EVERYWHERE (encoder and shoutcast classes)

#include "encodervorbis.h"

#include <stdlib.h> // needed for random num gen
#include <time.h> // needed for random num gen
#include <string.h> // needed for memcpy
#include <QDebug>

#include "engine/engineabstractrecord.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "playerinfo.h"
#include "trackinfoobject.h"

// Constructor
EncoderVorbis::EncoderVorbis(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine)
{
    pEngine = engine;
    metaDataTitle = metaDataArtist = NULL;
    m_pMetaData = TrackPointer(NULL);

    m_pConfig = _config;
}

// Destructor
EncoderVorbis::~EncoderVorbis()
{
  flushStream();
  vorbis_info_clear(&vinfo);
}

void EncoderVorbis::flushStream()
{
    vorbis_analysis_wrote(&vdsp, 0);
    sendPackages();

    ogg_stream_clear(&oggs);
    vorbis_block_clear(&vblock);
    vorbis_dsp_clear(&vdsp);
    vorbis_comment_clear(&vcomment);
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

// TODO: optimize this function a bit
bool EncoderVorbis::metaDataHasChanged()
{
    bool changed = false;

    // get active tracks
    int track = 0;
    if (ControlObject::getControl(ConfigKey("[Channel1]","play"))->get()==1.) track+=1;
    if (ControlObject::getControl(ConfigKey("[Channel2]","play"))->get()==1.) track+=2;

    switch (track)
    {
    case 0:
        // no tracks are playing
        break;
    case 1:
        // track 1 is playing
        {
        TrackPointer newMetaData = PlayerInfo::Instance().getTrackInfo(1);
        if (newMetaData != m_pMetaData)
        {
            m_pMetaData = newMetaData;
            changed = true;
        }
        }
        break;
    case 2:
        // track 2 is playing
        {
        TrackPointer newMetaData = PlayerInfo::Instance().getTrackInfo(2);
        if (newMetaData != m_pMetaData)
        {
            m_pMetaData = newMetaData;
            changed = true;
        }
        }
        break;
    case 3:
        // select most active track based on crossfader position
        ControlObjectThreadMain* m_pCrossfader = new ControlObjectThreadMain(
                                                     ControlObject::getControl(ConfigKey(
                                                     "[Master]","crossfader")));
        if (m_pCrossfader->get() <= 0)
        {
            TrackPointer newMetaData = PlayerInfo::Instance().getTrackInfo(1);
            if (newMetaData != m_pMetaData)
            {
                m_pMetaData = newMetaData;
                changed = true;
            }
        } else {
            TrackPointer newMetaData = PlayerInfo::Instance().getTrackInfo(2);
            if (newMetaData != m_pMetaData)
            {
                m_pMetaData = newMetaData;
                changed = true;
            }
        }
        delete m_pCrossfader;
        break;
    }
    return changed;
}

void EncoderVorbis::sendPackages()
{
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
                pEngine->writePage(oggpage.header, oggpage.body, oggpage.header_len, oggpage.body_len);
                if (ogg_page_eos(&oggpage)) eos = 1;
            }
        }
    }
}

void EncoderVorbis::encodeBuffer(const CSAMPLE *samples, const int size)
{
    float **buffer;
    int i;

    if (metaDataHasChanged() && m_pMetaData != NULL)
        updateMetaData(m_pMetaData);

    buffer = vorbis_analysis_buffer(&vdsp, size);

    // Deinterleave samples
    for (i = 0; i < size/2; ++i)
    {
        buffer[0][i] = samples[i*2]/32768.f;
        buffer[1][i] = samples[i*2+1]/32768.f;
    }

    vorbis_analysis_wrote(&vdsp, i);
    sendPackages();
}

void EncoderVorbis::updateMetaData(TrackPointer trackInfoObj)
{
    // convert QStrings to char*s
    QByteArray baArtist = m_pMetaData->getArtist().toLatin1();
    QByteArray baTitle = m_pMetaData->getTitle().toLatin1();
    metaDataArtist = baArtist.data();
    metaDataTitle = baTitle.data();

    flushStream();
    initStream();
}

void EncoderVorbis::initStream()
{
        // set up analysis state and auxiliary encoding storage
        vorbis_analysis_init(&vdsp, &vinfo);
        vorbis_block_init(&vdsp, &vblock);

        // set up packet-to-stream encoder; attach a random serial number
        srand(time(0));
        ogg_stream_init(&oggs, getSerial());

        // add comment
        vorbis_comment_init(&vcomment);
        vorbis_comment_add_tag(&vcomment, "ENCODER", "mixxx/libvorbis");
        if (metaDataArtist != NULL)
            vorbis_comment_add_tag(&vcomment, "ARTIST", metaDataArtist);
        if (metaDataTitle != NULL)
            vorbis_comment_add_tag(&vcomment, "TITLE", metaDataTitle);

        // set up the vorbis headers
        ogg_packet headerInit;
        ogg_packet headerComment;
        ogg_packet headerCode;
        vorbis_analysis_headerout(&vdsp, &vcomment, &headerInit, &headerComment, &headerCode);
        ogg_stream_packetin(&oggs, &headerInit);
        ogg_stream_packetin(&oggs, &headerComment);
        ogg_stream_packetin(&oggs, &headerCode);

        // start audio data on new page
        int result;
        while (1) {
            result = ogg_stream_flush(&oggs, &oggpage);
            if (result==0) break;
                  pEngine->writePage(oggpage.header, oggpage.body, oggpage.header_len, oggpage.body_len);
        }
}

// TODO: reinit encoder when samplerate or quality is updated

int EncoderVorbis::initEncoder(float quality)
{
    int ret;
    vorbis_info_init(&vinfo);

    // initialize VBR quality based mode
    unsigned long samplerate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();
    ret = vorbis_encode_init_vbr(&vinfo, 2, samplerate, quality);

    if (ret == 0) {
        initStream();
    } else {
        ret = -1;
    };
    return ret;
}

int EncoderVorbis::initEncoder(int bitrate)
{
    int ret;
    vorbis_info_init(&vinfo);

    // initialize VBR quality based mode
    unsigned long samplerate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();
    ret = vorbis_encode_init(&vinfo, 2, samplerate, -1, bitrate*1000, -1);

    if (ret == 0) {
        initStream();
    } else {
        ret = -1;
    };
    return ret;
}

int EncoderVorbis::initEncoder()
{
    return this->initEncoder((float)0.4);
}
