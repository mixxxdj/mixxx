/****************************************************************************
                   encoderffmpegvorbis.cpp  -  FFMPEG OGG/Vorbis encoder for mixxx
                             -------------------
    copyright            : (C) 2012-2013 by Tuukka Pasanen
                           (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) 1994 Tobias Rafreider (broadcast and recording fixes)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENCODERFFMPEGVORBIS_H
#define ENCODERFFMPEGVORBIS_H

#include "encoderffmpegcore.h"

class EncoderFfmpegCore;

class EncoderFfmpegVorbis : public EncoderFfmpegCore {
public:
    EncoderFfmpegVorbis(EncoderCallback* pCallback=NULL);
};

#endif
