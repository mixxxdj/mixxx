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


class SoundSourceFFmpeg : public Mixxx::SoundSource {
public:
    SoundSourceFFmpeg(QString qFilename);
    ~SoundSourceFFmpeg();
    int open();
    long seek(long);
    unsigned int read(unsigned long size, const SAMPLE*);
    Result parseHeader();
    inline long unsigned length();
    bool readInput();
    static QList<QString> supportedFileExtensions();
    AVCodecContext *getCodecContext();
    AVFormatContext *getFormatContext();
    int getAudioStreamIndex();
    double convertPtsToByteOffset(double pts, const AVRational &ffmpegtime);
    double convertByteOffsetToPts(double byteoffset, const AVRational &ffmpegtime);

protected:
    int64_t convertPtsToByteOffsetOld(int64_t pos, const AVRational &time_base);
    int64_t convertByteOffsetToPtsOld(int64_t pos, const AVRational &time_base);
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

    int64_t m_iOffset;
    int64_t m_iSeekOffset;
    int64_t m_iCurrentMixxTs;
    int64_t m_iLastFirstFfmpegByteOffset;
    int64_t m_iNextMixxxPCMPoint;
    bool m_bIsSeeked;
    int64_t m_iReadedBytes;
    QByteArray m_strBuffer;

    double_t m_fMixxBytePosition;

};

#endif
