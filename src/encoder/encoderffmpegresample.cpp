/****************************************************************************
                   encoderffmpegcore.cpp  -  FFMPEG encoder for mixxx
                             -------------------
    copyright            : (C) 2012-2013 by Tuukka Pasanen
                           (C) 2007 by Wesley Stessens
                           (C) 1994 by Xiph.org (encoder example)
                           (C) 1994 Tobias Rafreider (shoutcast and recording fixes)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "encoder/encoderffmpegresample.h"
#include "sampleutil.h"

EncoderFfmpegResample::EncoderFfmpegResample(AVCodecContext *codecCtx) {
    m_pCodecCtx = codecCtx;
}

EncoderFfmpegResample::~EncoderFfmpegResample() {
}

int EncoderFfmpegResample::openMixxx(enum AVSampleFormat inSampleFmt,
                                     enum AVSampleFormat outSampleFmt) {
    m_pOutSampleFmt = outSampleFmt;
    m_pInSampleFmt = inSampleFmt;

    qDebug() << "EncoderFfmpegResample::openMixxx: open MIXXX FFmpeg Resampler version";

    qDebug() << "Created sample rate converter for conversion of" <<
             m_pCodecCtx->sample_rate << "Hz format:" <<
             av_get_sample_fmt_name(inSampleFmt)
             << "with:" <<  (int)m_pCodecCtx->channels << "(layout:" <<
             m_pCodecCtx->channel_layout << ") channels (BPS"
             << av_get_bytes_per_sample(
                 m_pCodecCtx->sample_fmt) << ")";
    qDebug() << "To " << m_pCodecCtx->sample_rate << " HZ format:" <<
             av_get_sample_fmt_name(outSampleFmt) << "with " << (int)m_pCodecCtx->channels << " (layout:" <<
             m_pCodecCtx->channel_layout << ") channels (BPS " <<
             av_get_bytes_per_sample(outSampleFmt) << ")";

    return 0;
}

unsigned int EncoderFfmpegResample::reSampleMixxx(AVFrame *inframe, quint8 **outbuffer) {
    quint8 *l_ptrBuf = NULL;
    bool l_bSupported = false;
    qint64 l_lInReadBytes = av_samples_get_buffer_size(NULL, m_pCodecCtx->channels,
                            inframe->nb_samples,
                            m_pCodecCtx->sample_fmt, 1);

    qint64 l_lOutReadBytes = av_samples_get_buffer_size(NULL, m_pCodecCtx->channels,
                             inframe->nb_samples,
                             m_pOutSampleFmt, 1);

    // This is Cap frame or very much broken!
    // So return before something goes bad
    if (inframe->nb_samples <= 0) {
        qDebug() << "EncoderFfmpegResample::reSample: nb_samples is zero";
        return 0;
    }

    if (l_lInReadBytes < 0) {
        return 0;
    }
    l_ptrBuf = (quint8 *)av_malloc(l_lOutReadBytes);

    switch(m_pCodecCtx->sample_fmt) {
#if LIBAVCODEC_VERSION_INT >= 3482368
        case AV_SAMPLE_FMT_FLTP: {
            SampleUtil::interleaveBuffer((CSAMPLE *)l_ptrBuf, (CSAMPLE *)inframe->data[0],
                                        (CSAMPLE *)inframe->data[1], l_lOutReadBytes / 8);
            outbuffer[0] = l_ptrBuf;
            l_bSupported = true;
        }
        break;
        case AV_SAMPLE_FMT_S16P: {
            quint8 *l_ptrConversion = (quint8 *)av_malloc(l_lInReadBytes);
            quint16 *l_ptrSrc1 = (quint16 *) inframe->data[0];
            quint16 *l_ptrSrc2 = (quint16 *) inframe->data[1];
            quint16 *l_ptrDest = (quint16 *) l_ptrConversion;

            // note: LOOP VECTORIZED.
            for (int i = 0; i < (l_lInReadBytes / 4); ++i) {
                l_ptrDest[2 * i] = l_ptrSrc1[i];
                l_ptrDest[2 * i + 1] = l_ptrSrc2[i];
            }

            SampleUtil::convertS16ToFloat32((CSAMPLE *)l_ptrBuf, (SAMPLE *)l_ptrConversion, l_lInReadBytes / 2);
            outbuffer[0] = l_ptrBuf;
            l_bSupported = true;
        }
        break;
#endif
        case AV_SAMPLE_FMT_FLT: {
            memcpy(l_ptrBuf, inframe->data[0], l_lInReadBytes);

            outbuffer[0] = l_ptrBuf;
            l_bSupported = true;
        }
        break;
        case AV_SAMPLE_FMT_S16: {
            SampleUtil::convertS16ToFloat32((CSAMPLE *)l_ptrBuf, (SAMPLE *)inframe->data[0], l_lInReadBytes / 2);
            outbuffer[0] = l_ptrBuf;
            l_bSupported = true;
        }
        break;

        case AV_SAMPLE_FMT_NONE:
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_DBL:
        case AV_SAMPLE_FMT_U8P:
#if LIBAVCODEC_VERSION_INT >= 3482368
        case AV_SAMPLE_FMT_S32P:
        case AV_SAMPLE_FMT_DBLP:
#endif
        case AV_SAMPLE_FMT_NB:
        default:
            qDebug() << "Unsupported sample format:" << av_get_sample_fmt_name(m_pCodecCtx->sample_fmt);
        break;
    }

    if(l_bSupported == true)
    {
        return l_lOutReadBytes;
    }

    // If conversion is unsupported still return silence to prevent crash
    memset(l_ptrBuf, 0x00, l_lOutReadBytes);
    outbuffer[0] = l_ptrBuf;
    return l_lOutReadBytes;
}

