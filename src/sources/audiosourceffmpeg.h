#ifndef AUDIOSOURCEFFMPEG_H
#define AUDIOSOURCEFFMPEG_H

#include "sources/audiosource.h"
#include "util/defs.h"

#include <encoder/encoderffmpegresample.h>

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

#include <QVector>

namespace Mixxx {

struct ffmpegLocationObject {
    quint64 pos;
    qint64 pts;
    quint64 startByte;
};

struct ffmpegCacheObject {
    quint64 startByte;
    quint32 length;
    quint8 *bytes;
};

class AudioSourceFFmpeg : public AudioSource {
public:
    static AudioSourcePointer create(QString fileName);

    ~AudioSourceFFmpeg();

    diff_type seekFrame(diff_type frameIndex) /*override*/;

    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

private:
    AudioSourceFFmpeg();

    Result open(QString fileName);

    void close();

    bool readFramesToCache(unsigned int count, qint64 offset);
    bool getBytesFromCache(char *buffer, quint64 offset, quint64 size);
    quint64 getSizeofCache();
    void clearCache() throw();

    unsigned int read(unsigned long size, SAMPLE*);

    AVFormatContext *m_pFormatCtx;
    int m_iAudioStream;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;

    EncoderFfmpegResample *m_pResample;

    qint64 m_iCurrentMixxTs;

    bool m_bIsSeeked;

    quint64 m_lCacheBytePos;
    quint64 m_lCacheStartByte;
    quint64 m_lCacheEndByte;
    quint32 m_lCacheLastPos;
    QVector<struct ffmpegCacheObject  *> m_SCache;
    QVector<struct ffmpegLocationObject  *> m_SJumpPoints;
    quint64 m_lLastStoredPos;
    qint64 m_lStoredSeekPoint;
};

}

#endif
