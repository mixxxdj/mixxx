/****************************************************************************
                   encoderffmpegopus.cpp  -  FFMPEG OPUS encoder for mixxx
                             -------------------
    copyright            : (C) 2012-2013 by Tuukka Pasanen
                           (C) 2007 by Wesley Stessens
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

#ifndef ENCODERFFMPEGOPUS_H
#define ENCODERFFMPEGOPUS_H


#include "encoderffmpegcore.h"

class EncoderFfmpegCore;

class EncoderFfmpegOpus : public EncoderFfmpegCore {
public:
    EncoderFfmpegOpus(EncoderCallback* pCallback=NULL);
};

#endif
