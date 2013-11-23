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

#ifndef SOUNDSOURCEFFMPEG_H
#define SOUNDSOURCEFFMPEG_H

#include <qstring.h>
#include "soundsource.h"
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>

class TrackInfoObject;


class SoundSourceFFmpeg : public SoundSource {
public:
    SoundSourceFFmpeg(QString qFilename);
    ~SoundSourceFFmpeg();
    long seek(long);
    unsigned read(unsigned long size, const SAMPLE*);
    inline long unsigned length();
    static int ParseHeader(TrackInfoObject * );
    bool readInput();

protected:
  long ffmpeg2mixxx(long pos, const AVRational &time_base);
  long mixxx2ffmpeg(long pos, const AVRational &time_base);
  void lock();
  void unlock();

private:
    int channels;
    unsigned long filelength;
    AVFormatContext *pFormatCtx;
    AVInputFormat *iformat;
    int audioStream;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame;
    AVPacket packet;

    volatile int bufferOffset;
    volatile int bufferSize;
    SAMPLE buffer[AVCODEC_MAX_AUDIO_FRAME_SIZE];
};

#endif
