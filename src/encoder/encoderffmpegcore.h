/***************************************************************************
                     encodervorbis.h  -  vorbis encoder for mixxx
                             -------------------
    copyright            : (C) 2012-2013 by Tuukka Pasanen
                           (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) 1994 Tobias Rafreider (broadcast and recording fixes)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENCODERFFMPEGCORE_H
#define ENCODERFFMPEGCORE_H

#include <encoder/encoderffmpegresample.h>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/common.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>

#ifndef __FFMPEGOLDAPI__
#include <libavutil/avutil.h>
#endif

// Compability
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

#include <QByteArray>
#include <QBuffer>

#include <QLibrary>

#include "util/types.h"
#include "encoder/encoder.h"
#include "track/track.h"

class EncoderCallback;

class EncoderFfmpegCore : public Encoder {
public:
#if LIBAVCODEC_VERSION_INT > 3544932
    EncoderFfmpegCore(EncoderCallback* pCallback=NULL,
                      AVCodecID codec = AV_CODEC_ID_MP2);
#else
    EncoderFfmpegCore(EncoderCallback* pCallback=NULL,
                      CodecID codec = CODEC_ID_MP2);
#endif
    ~EncoderFfmpegCore();
    int initEncoder(int bitrate, int samplerate);
    void encodeBuffer(const CSAMPLE *samples, const int size);
    void updateMetaData(char* artist, char* title, char* album);
    void flush();
protected:
    unsigned int reSample(AVFrame *inframe);


private:
    int getSerial();
    bool metaDataHasChanged();
    //Call this method in conjunction with broadcast streaming
    int writeAudioFrame(AVFormatContext *oc, AVStream *st);
    void closeAudio(AVStream *st);
    void openAudio(AVCodec *codec, AVStream *st);
#if LIBAVCODEC_VERSION_INT > 3544932
    AVStream *addStream(AVFormatContext *oc, AVCodec **codec,
                        enum AVCodecID codec_id);
#else
    AVStream *addStream(AVFormatContext *oc, AVCodec **codec,
                        enum CodecID codec_id);
#endif
    bool m_bStreamInitialized;

    EncoderCallback* m_pCallback;
    TrackPointer m_pMetaData;

    char *m_strMetaDataTitle;
    char *m_strMetaDataArtist;
    char *m_strMetaDataAlbum;
    QFile m_pFile;

    QByteArray m_strReadByteArray;
    CSAMPLE m_SBuffer[65535];
    unsigned long m_lBufferSize;

    AVFormatContext *m_pEncodeFormatCtx;
    AVStream *m_pEncoderAudioStream;
    AVCodec *m_pEncoderAudioCodec;
    AVOutputFormat *m_pEncoderFormat;

    uint8_t *m_pSamples;
    float *m_pFltSamples;
    int m_iAudioInputFrameSize;

    unsigned int m_iFltAudioCpyLen;
    unsigned int m_iAudioCpyLen;

    uint32_t m_lBitrate;
    uint32_t m_lSampleRate;
    uint64_t m_lRecordedBytes;
    uint64_t m_lDts;
    uint64_t m_lPts;
#if LIBAVCODEC_VERSION_INT > 3544932
    enum AVCodecID m_SCcodecId;
#else
    enum CodecID m_SCcodecId;
#endif

    EncoderFfmpegResample *m_pResample;
    AVStream *m_pStream;
};

#endif
