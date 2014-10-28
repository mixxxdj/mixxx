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

#ifndef __FFMPEGOLDAPI__

// Thank you macports not providing libavresample for FFMPEG!
#ifdef __LIBAVRESAMPLE__
#include <libavresample/avresample.h>
#else
#include <libswresample/swresample.h>
#endif

#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#endif

// Compability
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>

}

class EncoderFfmpegResample {
public:
    EncoderFfmpegResample(AVCodecContext *codecCtx);
    ~EncoderFfmpegResample();
    int open(enum AVSampleFormat inSampleFmt, enum AVSampleFormat outSampleFmt);

    unsigned int reSample(AVFrame *inframe, quint8 **outbuffer);

private:
    AVCodecContext *m_pCodecCtx;
    enum AVSampleFormat m_pOutSampleFmt;
    enum AVSampleFormat m_pInSampleFmt;

#ifndef __FFMPEGOLDAPI__

// Please choose to use libavresample.. people
// Compile it now.. but because macports doesn't
// Support both.. damn!
#ifdef __LIBAVRESAMPLE__
    AVAudioResampleContext *m_pSwrCtx;
#else
    SwrContext *m_pSwrCtx;
#endif

#else
    ReSampleContext *m_pSwrCtx;
#endif

};

#endif
