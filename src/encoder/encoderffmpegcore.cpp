#include "encoder/encoderffmpegcore.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <QtDebug>

#include "encoder/encodercallback.h"
#include "errordialoghandler.h"


//
// FFMPEG changed their variable/define names in 1.0
// smallest number that is AV_/AV compatible avcodec version is
// 54/59/100 which is 3554148
//

// Constructor
#if LIBAVCODEC_VERSION_INT > 3544932
EncoderFfmpegCore::EncoderFfmpegCore(EncoderCallback* pCallback,
                                     AVCodecID codec)
#else
EncoderFfmpegCore::EncoderFfmpegCore(EncoderCallback* pCallback, CodecID codec)
#endif
{
    m_bStreamInitialized = false;
    m_pCallback = pCallback;

    m_pEncodeFormatCtx = NULL;
    m_pEncoderAudioStream = NULL;
    m_pEncoderAudioCodec = NULL;
    m_pEncoderFormat = NULL;


    m_pSamples = NULL;
    m_pFltSamples = NULL;
    m_iAudioInputFrameSize = -1;

    memset(m_SBuffer, 0x00, (65355 * 2));
    m_lBufferSize = 0;
    m_iAudioCpyLen = 0;
    m_iFltAudioCpyLen = 0;

    m_SCcodecId = codec;
    m_lBitrate = 128000;
    m_lDts = 0;
    m_lPts = 0;
    m_lRecordedBytes = 0;

}

// Destructor  //call flush before any encoder gets deleted
EncoderFfmpegCore::~EncoderFfmpegCore() {

    qDebug() << "EncoderFfmpegCore::~EncoderFfmpegCore()";
    av_freep(&m_pSamples);
    av_freep(&m_pFltSamples);

    avio_open_dyn_buf(&m_pEncodeFormatCtx->pb);
    if (av_write_trailer(m_pEncodeFormatCtx) != 0) {
        qDebug() << "Multiplexer: failed to write a trailer.";
    } else {
        unsigned char *l_strBuffer = NULL;
        int l_iBufferLen = 0;
        l_iBufferLen = avio_close_dyn_buf(m_pEncodeFormatCtx->pb,
                                          (uint8_t**)(&l_strBuffer));
        m_pCallback->write(NULL, l_strBuffer, 0, l_iBufferLen);
        av_free(l_strBuffer);
    }


    if (m_pStream != NULL) {
        avcodec_close(m_pStream->codec);
    }

    if (m_pEncodeFormatCtx != NULL) {
        av_free(m_pEncodeFormatCtx);
    }

    // Close buffer
    delete m_pResample;
}

void EncoderFfmpegCore::setEncoderSettings(const EncoderSettings& settings)
{
    m_lBitrate = settings.getQuality() * 1000;
}

//call sendPackages() or write() after 'flush()' as outlined in enginebroadcast.cpp
void EncoderFfmpegCore::flush() {
}


//  Get new random serial number
//  -> returns random number

int EncoderFfmpegCore::getSerial() {
    int l_iSerial = 0;
    return l_iSerial;
}

void EncoderFfmpegCore::encodeBuffer(const CSAMPLE *samples, const int size) {
    unsigned char *l_strBuffer = NULL;
    int l_iBufferLen = 0;
    //int l_iAudioCpyLen = m_iAudioInputFrameSize *
    //                     av_get_bytes_per_sample(m_pEncoderAudioStream->codec->sample_fmt) *
    //                     m_pEncoderAudioStream->codec->channels;
    long l_iLeft = size;
    long j = 0;
    unsigned int l_iBufPos = 0;
    unsigned int l_iPos = 0;

    // TODO(XXX): Get rid of repeated malloc here!
    float *l_fNormalizedSamples = (float *)malloc(size * sizeof(float));

    // We use normalized floats in the engine [-1.0, 1.0] and FFMPEG expects
    // samples in the range [-1.0, 1.0] so no conversion is required.
    for (j = 0; j < size; j++) {
        l_fNormalizedSamples[j] = samples[j];
    }

    // In MP3 this writes Header same In ogg
    // They are written once front of the encoded stuff
    if (m_bStreamInitialized == false) {
        m_bStreamInitialized = true;
        // Write a header.
        avio_open_dyn_buf(&m_pEncodeFormatCtx->pb);
        if (avformat_write_header(m_pEncodeFormatCtx, NULL) != 0) {
            qDebug() << "EncoderFfmpegCore::encodeBuffer: failed to write a header.";
            return;
        }

        l_iBufferLen = avio_close_dyn_buf(m_pEncodeFormatCtx->pb,
                                          (uint8_t**)(&l_strBuffer));
        m_pCallback->write(NULL, l_strBuffer, 0, l_iBufferLen);
        av_free(l_strBuffer);
    }

    while (l_iLeft > (m_iFltAudioCpyLen / 4)) {
        memset(m_pFltSamples, 0x00, m_iFltAudioCpyLen);

        for (j = 0; j < m_iFltAudioCpyLen / 4; j++) {
            if (m_lBufferSize > 0) {
                m_pFltSamples[j] = m_SBuffer[ l_iBufPos++ ];
                m_lBufferSize--;
                m_lRecordedBytes++;
            } else {
                m_pFltSamples[j] = l_fNormalizedSamples[l_iPos++];
                l_iLeft--;
                m_lRecordedBytes++;
            }

            if (l_iLeft <= 0) {
                qDebug() << "ffmpegencodercore: No samples left.. for encoding!";
                break;
            }
        }

        m_lBufferSize = 0;

        // Open dynamic buffer for writing next bytes
        if (avio_open_dyn_buf(&m_pEncodeFormatCtx->pb) < 0) {
            qDebug() << "Can't alloc Dyn buffer!";
            return;
        }

        // Write it to buffer (FILE) and then close buffer for waiting
        // Next encoded buffe to come or we stop encode
        if (! writeAudioFrame(m_pEncodeFormatCtx, m_pEncoderAudioStream)) {
            l_iBufferLen = avio_close_dyn_buf(m_pEncodeFormatCtx->pb,
                                              (uint8_t**)(&l_strBuffer));
            m_pCallback->write(NULL, l_strBuffer, 0, l_iBufferLen);
            av_free(l_strBuffer);
        }
    }

    // Keep things clean
    memset(m_SBuffer, 0x00, 65535);

    for (j = 0; j < l_iLeft; j++) {
        m_SBuffer[ j ] = l_fNormalizedSamples[ l_iPos++ ];
    }
    m_lBufferSize = l_iLeft;
    free(l_fNormalizedSamples);
}

// Originally called from enginebroadcast.cpp to update metadata information
// when streaming, however, this causes pops
//
// Currently this method is used before init() once to save artist, title and album
//
void EncoderFfmpegCore::updateMetaData(const QString& artist, const QString& title, const QString& album) {
    qDebug() << "ffmpegencodercore: UpdateMetadata: !" << artist << " - " << title <<
             " - " << album;
    m_strMetaDataTitle = title;
    m_strMetaDataArtist = artist;
    m_strMetaDataAlbum = album;
}

int EncoderFfmpegCore::initEncoder(int samplerate, QString* pUserErrorMessage) {
#ifndef avformat_alloc_output_context2
    qDebug() << "EncoderFfmpegCore::initEncoder: Old Style initialization";
    m_pEncodeFormatCtx = avformat_alloc_context();
#endif

    m_lSampleRate = samplerate;
    QString codecString;

#if LIBAVCODEC_VERSION_INT > 3544932
    if (m_SCcodecId == AV_CODEC_ID_MP3) {
#else
    if (m_SCcodecId == CODEC_ID_MP3) {
#endif // LIBAVCODEC_VERSION_INT > 3544932
        qDebug() << "EncoderFfmpegCore::initEncoder: Codec MP3";
        codecString = "Codec MP3";
#ifdef avformat_alloc_output_context2
        avformat_alloc_output_context2(&m_pEncodeFormatCtx, NULL, NULL, "output.mp3");
#else
        m_pEncoderFormat = av_guess_format(NULL, "output.mp3", NULL);
#endif // avformat_alloc_output_context2

#if LIBAVCODEC_VERSION_INT > 3544932
    } else if (m_SCcodecId == AV_CODEC_ID_AAC) {
#else
    } else if (m_SCcodecId == CODEC_ID_AAC) {
#endif // LIBAVCODEC_VERSION_INT > 3544932
        qDebug() << "EncoderFfmpegCore::initEncoder: Codec M4A";
        codecString = "Codec M4A";
#ifdef avformat_alloc_output_context2
        avformat_alloc_output_context2(&m_pEncodeFormatCtx, NULL, NULL, "output.m4a");
#else
        m_pEncoderFormat = av_guess_format(NULL, "output.m4a", NULL);
#endif // avformat_alloc_output_context2

    } else {
        qDebug() << "EncoderFfmpegCore::initEncoder: Codec OGG/Vorbis";
        codecString = "Codec OGG/Vorbis";
#ifdef avformat_alloc_output_context2
        avformat_alloc_output_context2(&m_pEncodeFormatCtx, NULL, NULL, "output.ogg");
        m_pEncodeFormatCtx->oformat->audio_codec=AV_CODEC_ID_VORBIS;
#else
        m_pEncoderFormat = av_guess_format(NULL, "output.ogg", NULL);
#if LIBAVCODEC_VERSION_INT > 3544932
        m_pEncoderFormat->audio_codec=AV_CODEC_ID_VORBIS;
#else
        m_pEncoderFormat->audio_codec=CODEC_ID_VORBIS;
#endif // LIBAVCODEC_VERSION_INT > 3544932
#endif // avformat_alloc_output_context2
    }

#ifdef avformat_alloc_output_context2
    m_pEncoderFormat = m_pEncodeFormatCtx->oformat;
#else
    m_pEncodeFormatCtx->oformat = m_pEncoderFormat;
#endif // avformat_alloc_output_context2

    m_pEncoderAudioStream = addStream(m_pEncodeFormatCtx, &m_pEncoderAudioCodec,
                                      m_pEncoderFormat->audio_codec);

    int ret = openAudio(m_pEncoderAudioCodec, m_pEncoderAudioStream);

    if (ret != 0) {
        errorMessage  = codecString + "recording is not supported. FFMPEG " 
                        + codecString + " could not be initialized";
        ret = -1;
    };

    return ret;
}

// Private methods

int EncoderFfmpegCore::writeAudioFrame(AVFormatContext *formatctx,
                                       AVStream *stream) {
    AVCodecContext *l_SCodecCtx = NULL;;
    AVPacket l_SPacket;
#if LIBAVCODEC_VERSION_INT < 3617792
    AVFrame *l_SFrame = avcodec_alloc_frame();
#else
    AVFrame *l_SFrame = av_frame_alloc();
#endif
    int l_iGotPacket;
    int l_iRet;
    uint8_t *l_iOut = NULL;
#ifdef av_make_error_string
    char l_strErrorBuff[256];
#endif // av_make_error_string

    av_init_packet(&l_SPacket);
    l_SPacket.size = 0;
    l_SPacket.data = NULL;

    // Calculate correct DTS for FFMPEG
    m_lDts = round(((double)m_lRecordedBytes / (double)44100 / (double)2. *
                    (double)m_pEncoderAudioStream->time_base.den));
    m_lPts = m_lDts;

    l_SCodecCtx = stream->codec;
#ifdef av_make_error_string
    memset(l_strErrorBuff, 0x00, 256);
#endif // av_make_error_string

    l_SFrame->nb_samples = m_iAudioInputFrameSize;
    // Mixxx uses float (32 bit) samples..
    l_SFrame->format = AV_SAMPLE_FMT_FLT;
#ifndef __FFMPEGOLDAPI__
    l_SFrame->channel_layout = l_SCodecCtx->channel_layout;
#endif // __FFMPEGOLDAPI__

    l_iRet = avcodec_fill_audio_frame(l_SFrame,
                                      l_SCodecCtx->channels,
                                      AV_SAMPLE_FMT_FLT,
                                      (const uint8_t *)m_pFltSamples,
                                      m_iFltAudioCpyLen,
                                      1);

    if (l_iRet != 0) {
#ifdef av_make_error_string
        qDebug() << "Can't fill FFMPEG frame: error " << l_iRet << "String '" <<
                 av_make_error_string(l_strErrorBuff, 256, l_iRet) << "'" <<
                 m_iFltAudioCpyLen;
#endif // av_make_error_string
        qDebug() << "Can't refill 1st FFMPEG frame!";
        return -1;
    }

    // If we have something else than AV_SAMPLE_FMT_FLT we have to convert it
    // to something that fits..
    if (l_SCodecCtx->sample_fmt != AV_SAMPLE_FMT_FLT) {

        m_pResample->reSampleMixxx(l_SFrame, &l_iOut);
        // After we have turned our samples to destination
        // Format we must re-alloc l_SFrame.. it easier like this..
        // FFMPEG 2.2 3561060 anb beyond
#if LIBAVCODEC_VERSION_INT >= 3561060
        av_frame_unref(l_SFrame);
        av_frame_free(&l_SFrame);
// FFMPEG 0.11 and below
#elif LIBAVCODEC_VERSION_INT <= 3544932
        av_free(l_SFrame);
// FFMPEG 1.0 - 2.1
#else
        avcodec_free_frame(&l_SFrame);
#endif
        l_SFrame = NULL;

#if LIBAVCODEC_VERSION_INT < 3617792
        l_SFrame = avcodec_alloc_frame();
#else
        l_SFrame = av_frame_alloc();
#endif
        l_SFrame->nb_samples = m_iAudioInputFrameSize;
        l_SFrame->format = l_SCodecCtx->sample_fmt;
#ifndef __FFMPEGOLDAPI__
        l_SFrame->channel_layout = m_pEncoderAudioStream->codec->channel_layout;
#endif // __FFMPEGOLDAPI__

        l_iRet = avcodec_fill_audio_frame(l_SFrame, l_SCodecCtx->channels,
                                          l_SCodecCtx->sample_fmt,
                                          l_iOut,
                                          m_iAudioCpyLen,
                                          1);

        free(l_iOut);
        l_iOut = NULL;

        if (l_iRet != 0) {
#ifdef av_make_error_string
            qDebug() << "Can't refill FFMPEG frame: error " << l_iRet << "String '" <<
                     av_make_error_string(l_strErrorBuff, 256,
                                          l_iRet) << "'" <<  m_iAudioCpyLen <<
                     " " <<  av_samples_get_buffer_size(
                         NULL, 2,
                         m_iAudioInputFrameSize,
                         m_pEncoderAudioStream->codec->sample_fmt,
                         1) << " " << m_pOutSize;
#endif // av_make_error_string
            qDebug() << "Can't refill 2nd FFMPEG frame!";
            return -1;
        }
    }

    //qDebug() << "!!" << l_iRet;
    l_iRet = avcodec_encode_audio2(l_SCodecCtx, &l_SPacket, l_SFrame,
                                   &l_iGotPacket);

    if (l_iRet < 0) {
        qDebug() << "Error encoding audio frame";
        return -1;
    }

    if (!l_iGotPacket) {
        // qDebug() << "No packet! Can't encode audio!!";
        return -1;
    }

    l_SPacket.stream_index = stream->index;

    // Let's calculate DTS/PTS and give it to FFMPEG..
    // THEN codecs like OGG/Voris works ok!!
    l_SPacket.dts = m_lDts;
    l_SPacket.pts = m_lDts;

#if LIBAVCODEC_VERSION_INT < 3617792
    // Some times den is zero.. so 0 dived by 0 is
    // Something?
    if (m_pEncoderAudioStream->pts.den == 0) {
        qDebug() << "Time hack!";
        m_pEncoderAudioStream->pts.den = 1;
    }
#endif

    // Write the compressed frame to the media file. */
    l_iRet = av_interleaved_write_frame(formatctx, &l_SPacket);

    if (l_iRet != 0) {
        qDebug() << "Error while writing audio frame";
        return -1;
    }

    av_free_packet(&l_SPacket);
#if LIBAVCODEC_VERSION_INT < 3617792
    av_destruct_packet(&l_SPacket);
#else
    av_free_packet(&l_SPacket);
#endif
    av_free(l_SFrame);

    return 0;
}


void EncoderFfmpegCore::closeAudio(AVStream *stream) {
    avcodec_close(stream->codec);
    av_free(m_pSamples);
}

int EncoderFfmpegCore::openAudio(AVCodec *codec, AVStream *stream) {
    AVCodecContext *l_SCodecCtx;
    int l_iRet;

    l_SCodecCtx = stream->codec;

    qDebug() << "openCodec!";

    // open it
    l_iRet = avcodec_open2(l_SCodecCtx, codec, NULL);
    if (l_iRet < 0) {
        qDebug() << "Could not open audio codec!";
        return -1;
    }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 89, 100)
    if (l_SCodecCtx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
#else
    if (l_SCodecCtx->codec->capabilities & CODEC_CAP_VARIABLE_FRAME_SIZE) {
#endif
        m_iAudioInputFrameSize = 10000;
    } else {
        m_iAudioInputFrameSize = l_SCodecCtx->frame_size;
    }

    m_iAudioCpyLen = m_iAudioInputFrameSize *
                     av_get_bytes_per_sample(stream->codec->sample_fmt) *
                     stream->codec->channels;


    m_iFltAudioCpyLen = av_samples_get_buffer_size(NULL, 2, m_iAudioInputFrameSize,
                        AV_SAMPLE_FMT_FLT,1);

    // m_pSamples is destination samples.. m_pFltSamples is FLOAT (32 bit) samples..
    m_pSamples = (uint8_t *)av_malloc(m_iAudioCpyLen * sizeof(uint8_t));
    //m_pFltSamples = (uint16_t *)av_malloc(m_iFltAudioCpyLen);
    m_pFltSamples = (float *)av_malloc(m_iFltAudioCpyLen * sizeof(float));

    if (!m_pSamples) {
        qDebug() << "Could not allocate audio samples buffer";
        return -2;
    }

    return 0;
}

// Add an output stream.
#if LIBAVCODEC_VERSION_INT > 3544932
AVStream *EncoderFfmpegCore::addStream(AVFormatContext *formatctx,
                                       AVCodec **codec, enum AVCodecID codec_id) {
#else
AVStream *EncoderFfmpegCore::addStream(AVFormatContext *formatctx,
                                       AVCodec **codec, enum CodecID codec_id) {
#endif
    AVCodecContext *l_SCodecCtx = NULL;
    AVStream *l_SStream = NULL;

    // find the encoder
    *codec = avcodec_find_encoder(codec_id);
    if (!(*codec)) {
#ifdef avcodec_get_name
        fprintf(stderr, "Could not find encoder for '%s'\n",
                avcodec_get_name(codec_id));
#endif
        return NULL;
    }

    l_SStream = avformat_new_stream(formatctx, *codec);
    if (!l_SStream) {
        qDebug() << "Could not allocate stream";
        return NULL;
    }
    l_SStream->id = formatctx->nb_streams-1;
    l_SCodecCtx = l_SStream->codec;

    m_pResample = new EncoderFfmpegResample(l_SCodecCtx);

    switch ((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        l_SStream->id = 1;
        l_SCodecCtx->sample_fmt = m_pEncoderAudioCodec->sample_fmts[0];

        l_SCodecCtx->bit_rate    = m_lBitrate;
        l_SCodecCtx->sample_rate = 44100;
        l_SCodecCtx->channels    = 2;

        m_pResample->openMixxx(AV_SAMPLE_FMT_FLT, l_SCodecCtx->sample_fmt);
        break;

    default:
        break;
    }


    // Some formats want stream headers to be separate.
    if (formatctx->oformat->flags & AVFMT_GLOBALHEADER) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 89, 100)
        l_SCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
#else
        l_SCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
#endif
    }

    return l_SStream;
}
