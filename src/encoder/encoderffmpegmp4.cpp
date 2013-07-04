/****************************************************************************
                   encoderffmpegmp4.cpp  -  FFMPEG MP4 encoder for mixxx
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

/*
   FFMPEG encoder class..
     - Supports what FFMPEG is compiled to supported
     - Same interface for all codecs
*/

#include "encoder/encoderffmpegmp4.h"

// Constructor
#ifdef AV_CODEC_ID_NONE
EncoderFfmpegMp4::EncoderFfmpegMp4(EncoderCallback* pCallback) : EncoderFfmpegCore( pCallback, AV_CODEC_ID_AAC )
#else
EncoderFfmpegMp4::EncoderFfmpegMp4(EncoderCallback* pCallback) : EncoderFfmpegCore( pCallback, CODEC_ID_AAC )
#endif
{
}

