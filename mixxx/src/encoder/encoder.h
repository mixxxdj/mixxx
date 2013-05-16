/****************************************************************************
                   encoder.h  - encoder API for mixxx
                             -------------------
    copyright            : (C) 2009 by Phillip Whelan
    copyright            : (C) 2010 by Tobias Rafreider
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENCODER_H
#define ENCODER_H

#include "defs.h"

class Encoder {
  public:
    Encoder();
    virtual ~Encoder();

    virtual int initEncoder(int bitrate, int samplerate) = 0;
    // encodes the provided buffer of audio.
    virtual void encodeBuffer(const CSAMPLE *samples, const int size) = 0;
    // Adds metadata to the encoded auio, i.e., the ID3 tag. Currently only used
    // by EngineRecord, EngineShoutcast does something different.
    virtual void updateMetaData(char* artist, char* title, char* album) = 0;
    // called at the end when encoding is finished
    virtual void flush() = 0;
    /**converts an OGG quality measure from 1..10 to a bitrate **/
    static int convertToBitrate(int quality);
};

#endif // ENCODER_H
