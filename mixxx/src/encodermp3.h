/****************************************************************************
                   encodermp3.h  - mp3 encoder for mixxx
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

#ifndef ENCODERMP3_H
#define ENCODERMP3_H

#include <QObject>
#include "defs.h"
#include "configobject.h"
#include "trackinfoobject.h"
#include "encoder.h"

#include <lame/lame.h> // may be elsewhere on other distros besides Ubuntu

class EngineAbstractRecord;

class EncoderMp3 : public Encoder {

public:
    EncoderMp3(ConfigObject<ConfigValue> *_config, EngineAbstractRecord *engine=0);
    ~EncoderMp3();
    int initEncoder();
    int initEncoder(float quality);
    int initEncoder(int bitrate);
    void encodeBuffer(const CSAMPLE *samples, const int size);

private:
    void flushStream();
    void initStream();
    int bufferOutGrow(int size);
    int bufferInGrow(int size);

    ConfigObject<ConfigValue> *m_pConfig; /* provides ConfigKey access */
    lame_global_flags *m_lameFlags;
    unsigned char *m_bufferOut;
    int m_bufferOutSize;
    float *m_bufferIn[2];
    int m_bufferInSize;

    EngineAbstractRecord *pEngine;
    TrackPointer m_pMetaData;
    char *metaDataTitle;
    char *metaDataArtist;
};

#endif
