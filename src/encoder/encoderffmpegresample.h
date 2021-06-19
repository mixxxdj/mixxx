#pragma once

#include <QtDebug>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

// Compatibility
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>

}

class EncoderFfmpegResample {
  public:
    explicit EncoderFfmpegResample(AVCodecContext *codecCtx);
    ~EncoderFfmpegResample();
    int openMixxx(AVSampleFormat inSampleFmt, AVSampleFormat outSampleFmt);

    unsigned int reSampleMixxx(AVFrame *inframe, quint8 **outbuffer);

  private:
    AVCodecContext *m_pCodecCtx;
    enum AVSampleFormat m_pOutSampleFmt;
    enum AVSampleFormat m_pInSampleFmt;

};
