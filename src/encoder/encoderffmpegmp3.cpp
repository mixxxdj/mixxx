/****************************************************************************
                   encoderffmpegmp3.cpp  -  FFMPEG MP3 encoder for mixxx
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

#include "encoder/encoderffmpegmp3.h"

// Constructor
#if LIBAVCODEC_VERSION_INT > 3544932
EncoderFfmpegMp3::EncoderFfmpegMp3(EncoderCallback* pCallback) : EncoderFfmpegCore(pCallback, AV_CODEC_ID_MP3)
#else
EncoderFfmpegMp3::EncoderFfmpegMp3(EncoderCallback* pCallback) : EncoderFfmpegCore(pCallback, CODEC_ID_MP3)
#endif
{
}
