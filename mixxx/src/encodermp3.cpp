/****************************************************************************
                   encodermp3.cpp  - mp3 encoder for mixxx
                             -------------------
    copyright            : (C) 2007 by Wesley Stessens
                           (C) 2009 by Phillip Whelan (rewritten for mp3)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h> // needed for random num     
#include <time.h> // needed for random num     
#include <string.h> // needed for memcpy
#include <QDebug>

#include "encodermp3.h"
#include "engine/engineabstractrecord.h"
#include "controlobjectthreadmain.h"
#include "controlobject.h"
#include "playerinfo.h"
#include "trackinfoobject.h"

// Constructor
EncoderMp3::EncoderMp3(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine)
{
    pEngine = engine;
    metaDataTitle = metaDataArtist = "";
    m_pMetaData = NULL;
    m_bufferIn[0] = NULL;
    m_bufferIn[1] = NULL;
    m_bufferOut = NULL;
    m_bufferOutSize = 0;
    m_lameFlags = NULL;
    m_pConfig = _config;
	/*
	 * @ Author: Tobias Rafreider
	 * Nobody has initialized the field before my code review.
     * At runtime the Integer field was inialized by a large random value
     * such that the following pointer fields were never initialized in the
	 * methods 'bufferOutGrow()' and 'bufferInGrow()' --> Valgrind shows invalid writes :-)
	 * 
     * m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
	 * m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
     * m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
	 *
     * This has solved many segfaults when using and even closing shoutcast along with LAME. 
	 * This bug was detected by using Valgrind memory analyser 
     *
    */
	m_bufferInSize = 0; 
}

// Destructor
EncoderMp3::~EncoderMp3()
{
    flushStream();
    lame_close(m_lameFlags);
	//free requested buffers
	if(m_bufferIn[0] != NULL) delete m_bufferIn[0];
    if(m_bufferIn[1] != NULL) delete m_bufferIn[1];
	if(m_bufferOut != NULL) delete m_bufferOut;
}

/*
 * Grow the outBuffer if needed.
 */

int EncoderMp3::bufferOutGrow(int size)
{
    if ( m_bufferOutSize >= size )
        return 0;
    
    m_bufferOut = (unsigned char *)realloc(m_bufferOut, size);
    if ( m_bufferOut == NULL )
        return -1;
    
    m_bufferOutSize = size;
    return 0;
}

/*
 * Grow the inBuffer(s) if needed.
 */

int EncoderMp3::bufferInGrow(int size)
{
    if ( m_bufferInSize >= size )
        return 0;
    
    m_bufferIn[0] = (float *)realloc(m_bufferIn[0], size * sizeof(float));
    m_bufferIn[1] = (float *)realloc(m_bufferIn[1], size * sizeof(float));
    if ((m_bufferIn[0] == NULL) || (m_bufferIn[1] == NULL))
        return -1;
    
    m_bufferInSize = size;
    return 0;
}

void EncoderMp3::flushStream()
{
    int rc;
    
    
    rc = lame_encode_flush_nogap(m_lameFlags, m_bufferOut, m_bufferOutSize);
    pEngine->writePage(NULL, m_bufferOut, 0, rc);
}

void EncoderMp3::encodeBuffer(const CSAMPLE *samples, const int size)
{
    int outsize;
    int rc;
    int i;
    
    
    outsize = (int)((1.25 * size + 7200) + 1);
    bufferOutGrow(outsize);
    
    bufferInGrow(size);
    
    // Deinterleave samples
    for (i = 0; i < size/2; ++i)
    {
        m_bufferIn[0][i] = samples[i*2];
        m_bufferIn[1][i] = samples[i*2+1];
    }
    
    rc = lame_encode_buffer_float(m_lameFlags, m_bufferIn[0], m_bufferIn[1], 
            size/2, m_bufferOut, m_bufferOutSize);
    if ( rc < 0 )
        return;
    
    pEngine->writePage(NULL, m_bufferOut, 0, rc);
}

void EncoderMp3::initStream()
{
    m_bufferOutSize = (int)((1.25 * 20000 + 7200) + 1);
    m_bufferOut = (unsigned char *)malloc(m_bufferOutSize);
    
    m_bufferIn[0] = (float *)malloc(m_bufferOutSize * sizeof(float));
    m_bufferIn[1] = (float *)malloc(m_bufferOutSize * sizeof(float));
    
    return;
}

int EncoderMp3::initEncoder(int bitrate)
{
    unsigned long samplerate = m_pConfig->getValueString(ConfigKey("[Soundcard]","Samplerate")).toULong();
    
    
    m_lameFlags = lame_init();
    if ( m_lameFlags == NULL ) {
        qDebug() << "Unable to initialize MP3";
        return -1;
    }
    
    lame_set_num_channels(m_lameFlags, 2);
    lame_set_in_samplerate(m_lameFlags, samplerate);
    lame_set_out_samplerate(m_lameFlags, samplerate);
    lame_set_brate(m_lameFlags, bitrate);
    lame_set_mode(m_lameFlags, STEREO);
    lame_set_quality(m_lameFlags, 2);
    lame_set_bWriteVbrTag(m_lameFlags, 0);
    
    
    if (( lame_init_params(m_lameFlags)) < 0) {
        qDebug() << "Unable to initialize MP3 parameters";
        return -1;
    }
    
    initStream();
    
    return 0;
}


