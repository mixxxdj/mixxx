/***************************************************************************
                     encoderffmpegcore.h  -  core code for ffmpeg encoders
                             -------------------
    copyright            : (C) 2012-2013 by Tuukka Pasanen
                           (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) ???? Tobias Rafreider (broadcast and recording fixes)
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

// Compatibility
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
}

#include <QBuffer>
#include <QByteArray>
#include <QLibrary>

#include "encoder/encoder.h"
#include "track/track_decl.h"
#include "util/types.h"

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
    int initEncoder(int samplerate, QString errorMessage) override;
    void encodeBuffer(const CSAMPLE *samples, const int size) override;
    void updateMetaData(const QString& artist, const QString& title, const QString& album) override;
    void flush() override;
    void setEncoderSettings(const EncoderSettings& settings) override;
protected:
    unsigned int reSample(AVFrame *inframe);


private:
    int getSerial();
    bool metaDataHasChanged();
    //Call this method in conjunction with broadcast streaming
    int writeAudioFrame(AVFormatContext *oc, AVStream *st);
    void closeAudio(AVStream *st);
    int openAudio(AVCodec *codec, AVStream *st);
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

    QString m_strMetaDataTitle;
    QString m_strMetaDataArtist;
    QString m_strMetaDataAlbum;
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
