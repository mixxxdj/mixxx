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

#include "soundsource.h"

#include <encoder/encoderffmpegresample.h>

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
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#endif

// Compability
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>

}
#define SOUNDSOURCEFFMPEGDEBUG

class TrackInfoObject;

struct ffmpegLocationObject {
    quint64 pos;
    qint64 pts;
    quint64 startByte;
};

struct ffmpegCacheObject {
    quint64 startByte;
    quint32 length;
    quint8 *bytes;
};

class SoundSourceFFmpeg : public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFFmpeg(QString qFilename);
    ~SoundSourceFFmpeg();

    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    bool readFramesToCache(unsigned int count, qint64 offset);
    bool getBytesFromCache(char *buffer, quint64 offset, quint64 size);
    quint64 getSizeofCache();
    bool clearCache();

    unsigned int read(unsigned long size, SAMPLE*);

    AVFormatContext *m_pFormatCtx;
    int m_iAudioStream;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;

    EncoderFfmpegResample *m_pResample;

    qint64 m_iCurrentMixxTs;

    bool m_bIsSeeked;

    quint64 m_lCacheBytePos;
    quint64 m_lCacheStartByte;
    quint64 m_lCacheEndByte;
    quint32 m_lCacheLastPos;
    QVector<struct ffmpegCacheObject  *> m_SCache;
    QVector<struct ffmpegLocationObject  *> m_SJumpPoints;
    quint64 m_lLastStoredPos;
    qint64 m_lStoredSeekPoint;
};

#endif
