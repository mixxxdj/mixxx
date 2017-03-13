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

EncoderFfmpegResample::EncoderFfmpegResample(AVCodecContext *codecCtx) {
    m_pSwrCtx = NULL;
    m_pCodecCtx = codecCtx;
}

EncoderFfmpegResample::~EncoderFfmpegResample() {
    if (m_pSwrCtx != NULL) {
#ifndef __FFMPEGOLDAPI__

#ifdef __LIBAVRESAMPLE__
        avresample_free(&m_pSwrCtx);
#else
        swr_free(&m_pSwrCtx);
#endif // __LIBAVRESAMPLE__

#else
        audio_resample_close(m_pSwrCtx);
#endif // __FFMPEGOLDAPI__
    }
}

int EncoderFfmpegResample::open(enum AVSampleFormat inSampleFmt,
                                enum AVSampleFormat outSampleFmt) {
    m_pOutSampleFmt = outSampleFmt;
    m_pInSampleFmt = inSampleFmt;

    // Some MP3/WAV don't tell this so make assumtion that
    // They are stereo not 5.1
    if (m_pCodecCtx->channel_layout == 0 && m_pCodecCtx->channels == 2) {
        m_pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
    } else if (m_pCodecCtx->channel_layout == 0 && m_pCodecCtx->channels == 1) {
        m_pCodecCtx->channel_layout = AV_CH_LAYOUT_MONO;
    } else if (m_pCodecCtx->channel_layout == 0 && m_pCodecCtx->channels == 0) {
        m_pCodecCtx->channel_layout = AV_CH_LAYOUT_STEREO;
        m_pCodecCtx->channels = 2;
    }


    // They make big change in FFPEG 1.1 before that every format just passed
    // s16 back to application from FFMPEG 1.1 up MP3 pass s16p (Planar stereo
    // 16 bit) MP4/AAC FLTP (Planar 32 bit float) and OGG also FLTP
    // (WMA same thing) If sample type aint' S16 (Stero 16) example FFMPEG 1.1
    // mp3 is s16p that ain't and mp4 FLTP (32 bit float)
    // NOT Going to work because MIXXX works with pure s16 that is not planar
    // GOOD thing is now this can handle allmost everything..
    // What should be tested is 44800 Hz downsample and 22100 Hz up sample.
    if ((inSampleFmt != outSampleFmt || m_pCodecCtx->sample_rate != 44100 ||
            m_pCodecCtx->channel_layout != AV_CH_LAYOUT_STEREO) && m_pSwrCtx == NULL) {
        if (m_pSwrCtx != NULL) {
            qDebug() << "Freeing Resample context";

// __FFMPEGOLDAPI__ Is what is used in FFMPEG < 0.10 and libav < 0.8.3. NO
// libresample available..
#ifndef __FFMPEGOLDAPI__

#ifdef __LIBAVRESAMPLE__
            avresample_close(m_pSwrCtx);
#else
            swr_free(&m_pSwrCtx);
#endif // __LIBAVRESAMPLE__

#else
            audio_resample_close(m_pSwrCtx);
#endif // __FFMPEGOLDAPI__
            m_pSwrCtx = NULL;
        }


        // Create converter from in type to s16 sample rate
#ifndef __FFMPEGOLDAPI__

#ifdef __LIBAVRESAMPLE__
        qDebug() << "ffmpeg: NEW FFMPEG API using libavresample";
        m_pSwrCtx = avresample_alloc_context();
#else
        qDebug() << "ffmpeg: NEW FFMPEG API using libswresample";
        m_pSwrCtx = swr_alloc();
#endif // __LIBAVRESAMPLE__

        av_opt_set_int(m_pSwrCtx,"in_channel_layout", m_pCodecCtx->channel_layout, 0);
        av_opt_set_int(m_pSwrCtx,"in_sample_fmt", inSampleFmt, 0);
        av_opt_set_int(m_pSwrCtx,"in_sample_rate", m_pCodecCtx->sample_rate, 0);
        av_opt_set_int(m_pSwrCtx,"out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(m_pSwrCtx,"out_sample_fmt", outSampleFmt, 0);
        av_opt_set_int(m_pSwrCtx,"out_sample_rate", m_pCodecCtx->sample_rate, 0);

#else
        qDebug() << "ffmpeg: OLD FFMPEG API in use!";
        m_pSwrCtx = av_audio_resample_init(2,
                                           m_pCodecCtx->channels,
                                           m_pCodecCtx->sample_rate,
                                           m_pCodecCtx->sample_rate,
                                           outSampleFmt,
                                           inSampleFmt,
                                           16,
                                           10,
                                           0,
                                           0.8);

#endif // __FFMPEGOLDAPI__
        if (!m_pSwrCtx) {
            qDebug() << "Can't init convertor!";
            return -1;
        }

#ifndef __FFMPEGOLDAPI__
        // If it not working let user know about it!
        // If we don't do this we'll gonna crash
#ifdef __LIBAVRESAMPLE__
        if (avresample_open(m_pSwrCtx) < 0) {
#else
        if (swr_init(m_pSwrCtx) < 0) {
#endif // __LIBAVRESAMPLE__
            m_pSwrCtx = NULL;
            qDebug() << "ERROR!! Conventor not created: " <<
                     m_pCodecCtx->sample_rate <<
                     "Hz " << av_get_sample_fmt_name(inSampleFmt) << " " <<
                     (int)m_pCodecCtx->channels << "(layout:" <<
                     m_pCodecCtx->channel_layout <<
                     ") channels";
            qDebug() << "To " << m_pCodecCtx->sample_rate << " HZ format:" <<
                     av_get_sample_fmt_name(outSampleFmt) << " with 2 channels";
            return -1;
        }
#endif // __FFMPEGOLDAPI__

        qDebug() << "Created sample rate converter for conversion of" <<
                 m_pCodecCtx->sample_rate << "Hz format:" <<
                 av_get_sample_fmt_name(inSampleFmt)
                 << "with:" <<  (int)m_pCodecCtx->channels << "(layout:" <<
                 m_pCodecCtx->channel_layout << ") channels (BPS"
                 << av_get_bytes_per_sample(
                     m_pCodecCtx->sample_fmt) << ")";
        qDebug() << "To " << m_pCodecCtx->sample_rate << " HZ format:" <<
                 av_get_sample_fmt_name(outSampleFmt) << "with 2 (layout:" <<
                 m_pCodecCtx->channel_layout << ") channels (BPS " <<
                 av_get_bytes_per_sample(outSampleFmt) << ")";
    }

    return 0;
}

unsigned int EncoderFfmpegResample::reSample(AVFrame *inframe, quint8 **outbuffer) {

    if (m_pSwrCtx) {

#ifndef __FFMPEGOLDAPI__

#ifdef __LIBAVRESAMPLE__
#if LIBAVRESAMPLE_VERSION_MAJOR == 0
        void **l_pIn = (void **)inframe->extended_data;
#else
        quint8 **l_pIn = (quint8 **)inframe->extended_data;
#endif // LIBAVRESAMPLE_VERSION_MAJOR == 0
#else
        quint8 **l_pIn = (quint8 **)inframe->extended_data;
#endif // __LIBAVRESAMPLE__

// Left here for reason!
// Sometime in time we will need this!
#else
        qint64 l_lInReadBytes = av_samples_get_buffer_size(NULL, m_pCodecCtx->channels,
                                 inframe->nb_samples,
                                 m_pCodecCtx->sample_fmt, 1);
#endif // __FFMPEGOLDAPI__

#ifndef __FFMPEGOLDAPI__
        int l_iOutBytes = 0;

#if __LIBAVRESAMPLE__
        int l_iOutSamples = av_rescale_rnd(avresample_get_delay(m_pSwrCtx) +
                                           inframe->nb_samples,
                                           m_pCodecCtx->sample_rate,
                                           m_pCodecCtx->sample_rate,
                                           AV_ROUND_UP);
#else
        int l_iOutSamples = av_rescale_rnd(swr_get_delay(m_pSwrCtx,
                                           m_pCodecCtx->sample_rate) +
                                           inframe->nb_samples,
                                           m_pCodecCtx->sample_rate,
                                           m_pCodecCtx->sample_rate,
                                           AV_ROUND_UP);
#endif // __LIBAVRESAMPLE__
        int l_iOutSamplesLines = 0;

        // Alloc too much.. if not enough we are in trouble!
        av_samples_alloc(outbuffer, &l_iOutSamplesLines, 2, l_iOutSamples,
                         m_pOutSampleFmt, 0);
#else
        int l_iOutSamples = av_rescale_rnd(inframe->nb_samples,
                                           m_pCodecCtx->sample_rate,
                                           m_pCodecCtx->sample_rate,
                                           AV_ROUND_UP);

        int l_iOutBytes =  av_samples_get_buffer_size(NULL, 2,
                           l_iOutSamples,
                           m_pOutSampleFmt, 1);


        outbuffer = (short *)malloc(l_iOutBytes * 2);
#endif // __FFMPEGOLDAPI__

        int l_iLen = 0;
#ifndef __FFMPEGOLDAPI__

#ifdef __LIBAVRESAMPLE__

// OLD API (0.0.3) ... still NEW API (1.0.0 and above).. very frustrating..
// USED IN FFMPEG 1.0 (LibAV SOMETHING!). New in FFMPEG 1.1 and libav 9
#if LIBAVRESAMPLE_VERSION_INT <= 3
        // AVResample OLD
        l_iLen = avresample_convert(m_pSwrCtx, (void **)outbuffer, 0, l_iOutSamples,
                                    (void **)l_pIn, 0, inframe->nb_samples);
#else
        //AVResample NEW
        l_iLen = avresample_convert(m_pSwrCtx, (quint8 **)outbuffer, 0, l_iOutSamples,
                                    (quint8 **)l_pIn, 0, inframe->nb_samples);
#endif // LIBAVRESAMPLE_VERSION_INT <= 3

#else
        // SWResample
        l_iLen = swr_convert(m_pSwrCtx, (quint8 **)outbuffer, l_iOutSamples,
                             (const quint8 **)l_pIn, inframe->nb_samples);
#endif // __LIBAVRESAMPLE__

        l_iOutBytes = av_samples_get_buffer_size(NULL, 2, l_iLen, m_pOutSampleFmt, 1);

#else
        l_iLen = audio_resample(m_pSwrCtx,
                                (short *)outbuffer, (short *)inframe->data[0],
                                inframe->nb_samples);

#endif // __FFMPEGOLDAPI__
        if (l_iLen < 0) {
            qDebug() << "Sample format conversion failed!";
            return -1;
        }
        return l_iOutBytes;
    } else {
        quint8 *l_ptrBuf = NULL;
        qint64 l_lInReadBytes = av_samples_get_buffer_size(NULL, m_pCodecCtx->channels,
                                 inframe->nb_samples,
                                 m_pCodecCtx->sample_fmt, 1);

        if(l_lInReadBytes < 0) {
           return 0;
        }

        l_ptrBuf = (quint8 *)av_malloc(l_lInReadBytes);

        memcpy(l_ptrBuf, inframe->data[0], l_lInReadBytes);

        outbuffer[0] = l_ptrBuf;
        return l_lInReadBytes;
    }

    return 0;
}
