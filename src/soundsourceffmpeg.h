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

#include <encoder/encoderffmpegresample.h>

#include <QString>
#include <QByteArray>
#include <QList>
#include <QVector>

#include "soundsource.h"
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
public:
    explicit SoundSourceFFmpeg(QString qFilename);
    ~SoundSourceFFmpeg();
    Result open();
    long seek(long);
    unsigned int read(unsigned long size, const SAMPLE*);
    Result parseHeader();
    QImage parseCoverArt();
    inline long unsigned length();
    bool readInput();
    static QList<QString> supportedFileExtensions();
    AVCodecContext *getCodecContext();
    AVFormatContext *getFormatContext();
    int getAudioStreamIndex();


protected:
    void lock();
    void unlock();

    bool readFramesToCache(unsigned int count, qint64 offset);
    bool getBytesFromCache(char *buffer, quint64 offset, quint64 size);
    quint64 getSizeofCache();
    bool clearCache();

private:
    int m_iAudioStream;
    quint64 m_filelength;
    AVFormatContext *m_pFormatCtx;
    AVInputFormat *m_pIformat;
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
