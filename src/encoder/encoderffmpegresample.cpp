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
// Avconv 9.x or above
#if LIBAVCODEC_VERSION_INT >= 3482368
    // Planar A.K.A Non-Interleaced version of samples
    // FFMPEG 1.2.x and above
    // Aconv 9.x and above
    if ( m_pCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP) {
        l_ptrBuf = (quint8 *)av_malloc(l_lOutReadBytes);

        SampleUtil::interleaveBuffer((CSAMPLE *)l_ptrBuf, (CSAMPLE *)inframe->data[0],
                                     (CSAMPLE *)inframe->data[1], l_lOutReadBytes / 8);
        outbuffer[0] = l_ptrBuf;
        return l_lOutReadBytes;
    } else if ( m_pCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P) {
        quint8 *l_ptrConversion = (quint8 *)av_malloc(l_lInReadBytes);
        l_ptrBuf = (quint8 *)av_malloc(l_lOutReadBytes);
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
        return l_lOutReadBytes;
    }
#endif

    // Backup if something really interesting is happening
    // or Mixxx is using old version of FFMPEG/Avconv via Ubuntu
    if ( m_pCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLT
       ) {
        l_ptrBuf = (quint8 *)av_malloc(l_lInReadBytes);

        memcpy(l_ptrBuf, inframe->data[0], l_lInReadBytes);

        outbuffer[0] = l_ptrBuf;
        return l_lInReadBytes;
    } else if ( m_pCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16) {
        l_ptrBuf = (quint8 *)av_malloc(l_lOutReadBytes);
        SampleUtil::convertS16ToFloat32((CSAMPLE *)l_ptrBuf, (SAMPLE *)inframe->data[0], l_lInReadBytes / 2);
        outbuffer[0] = l_ptrBuf;
        return l_lOutReadBytes;

    } else {
        qDebug() << "Unknow sample format:" << av_get_sample_fmt_name(m_pCodecCtx->sample_fmt) << av_get_sample_fmt_name(m_pOutSampleFmt);
    }

    return 0;
}

