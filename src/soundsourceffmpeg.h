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
//#include <QContiguousCache>
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
    uint64_t pos;
    int64_t pts;
    uint64_t startByte;
};

struct ffmpegCacheObject {
    uint64_t startByte;
    uint32_t length;
    uint8_t *bytes;
};

class SoundSourceFFmpeg : public Mixxx::SoundSource {
public:
    SoundSourceFFmpeg(QString qFilename);
    ~SoundSourceFFmpeg();
    Result open();
    long seek(long);
    unsigned int read(unsigned long size, const SAMPLE*);
    Result parseHeader();
    inline long unsigned length();
    bool readInput();
    static QList<QString> supportedFileExtensions();
    AVCodecContext *getCodecContext();
    AVFormatContext *getFormatContext();
    int getAudioStreamIndex();


protected:
    void lock();
    void unlock();

    bool readFramesToCache(unsigned int count, int64_t offset);
    bool getBytesFromCache(char *buffer, uint64_t offset, uint64_t size);
    uint64_t getSizeofCache();
    bool clearCache();

private:
    int m_iAudioStream;
    uint64_t filelength;
    QString m_qFilename;
    AVFormatContext *m_pFormatCtx;
    AVInputFormat *m_pIformat;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;

    int64_t m_iCurrentMixxTs;

    EncoderFfmpegResample *m_pResample;

    bool m_bIsSeeked;

    uint64_t m_lCacheBytePos;
    uint64_t m_lCacheStartByte;
    uint64_t m_lCacheEndByte;
    uint32_t m_lCacheLastPos;
    QVector<struct ffmpegCacheObject  *> m_SCache;
    QVector<struct ffmpegLocationObject  *> m_SJumpPoints;
    uint64_t m_lLastStoredPos;
    int64_t m_lStoredSeekPoint;
};

#endif
