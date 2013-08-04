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

#include <qstring.h>
#include <QByteArray>
#include <QBuffer>
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
class TrackInfoObject;


class SoundSourceFFmpeg : public Mixxx::SoundSource {
public:
    SoundSourceFFmpeg(QString qFilename);
    ~SoundSourceFFmpeg();
    int open();
    long seek(long);
    unsigned int read(unsigned long size, const SAMPLE*);
    int parseHeader();
    inline long unsigned length();
    //static int ParseHeader(TrackInfoObject * );
    bool readInput();
    static QList<QString> supportedFileExtensions();

protected:
    int64_t ffmpeg2mixxx(int64_t pos, const AVRational &time_base);
    int64_t mixxx2ffmpeg(int64_t pos, const AVRational &time_base);
    void lock();
    void unlock();

private:
    int m_iAudioStream;
    uint64_t filelength;
    QString m_qFilename;
    AVFormatContext *m_pFormatCtx;
    AVInputFormat *m_pIformat;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;

    EncoderFfmpegResample *m_pResample;

    unsigned int m_iOffset;
    int64_t m_iCurrentMixxTs;
    int64_t m_iNextMixxxPCMPoint;
    bool m_bIsSeeked;
    int64_t m_iReadedBytes;
    QByteArray m_strBuffer;

};

#endif
