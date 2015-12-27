/* -*- mode:C++; indent-tabs-mode:t; tab-width:8; c-basic-offset:4; -*- */
/***************************************************************************
                          soundsourceffmpeg.h  -  ffmpeg decoder
                             -------------------
    copyright            : (C) 2003 by Cedric GESTES
    email                : goctaf@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENCODERFFMPEGRESAMPLE_H
#define ENCODERFFMPEGRESAMPLE_H

#include <QtDebug>

extern "C" {
// Needed to ensure that macros in <stdint.h> get defined.
#ifndef __STDC_CONSTANT_MACROS
#if __cplusplus < 201103L
#define __STDC_CONSTANT_MACROS
#endif
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

// Compability
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>

}

class EncoderFfmpegResample {
  public:
    EncoderFfmpegResample(AVCodecContext *codecCtx);
    ~EncoderFfmpegResample();
    int openMixxx(enum AVSampleFormat inSampleFmt, enum AVSampleFormat outSampleFmt);

    unsigned int reSampleMixxx(AVFrame *inframe, quint8 **outbuffer);

  private:
    AVCodecContext *m_pCodecCtx;
    enum AVSampleFormat m_pOutSampleFmt;
    enum AVSampleFormat m_pInSampleFmt;

};

#endif
