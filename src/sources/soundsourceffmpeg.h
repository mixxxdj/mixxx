#ifndef MIXXX_SOUNDSOURCEFFMPEG_H
#define MIXXX_SOUNDSOURCEFFMPEG_H

#include "sources/soundsource.h"

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
    SINT pos;
    SINT pts;
    SINT startFrame;
};

struct ffmpegCacheObject {
    SINT startFrame;
    SINT length;
    quint8 *bytes;
};

class SoundSourceFFmpeg : public SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceFFmpeg(QUrl url);
    ~SoundSourceFFmpeg();

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames, CSAMPLE* sampleBuffer) /*override*/;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) /*override*/;

    bool readFramesToCache(unsigned int count, SINT offset);
    bool getBytesFromCache(char *buffer, SINT offset, SINT size);
    SINT getSizeofCache();
    void clearCache();

    unsigned int read(unsigned long size, SAMPLE*);

<<<<<<< HEAD
    Result parseMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;
    QImage parseCoverArt() const /*override*/;

    Mixxx::AudioSourcePointer open() const /*override*/;
=======
    AVFormatContext *m_pFormatCtx;
    int m_iAudioStream;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;

    EncoderFfmpegResample *m_pResample;

    SINT m_currentMixxxFrameIndex;

    bool m_bIsSeeked;

    SINT m_lCacheFramePos;
    SINT m_lCacheStartFrame;
    SINT m_lCacheEndFrame;
    SINT m_lCacheLastPos;
    QVector<struct ffmpegCacheObject  *> m_SCache;
    QVector<struct ffmpegLocationObject  *> m_SJumpPoints;
<<<<<<< HEAD
    quint64 m_lLastStoredPos;
    qint64 m_lStoredSeekPoint;
>>>>>>> Move code from specialized AudioSources back into corresponding SoundSources
=======
    SINT m_lLastStoredPos;
    SINT m_lStoredSeekPoint;
>>>>>>> Get rid of Byte word in variables (use Frame) in SoundSourceFFMPEG and change everything to SINT which seems to be preferred type
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEFFMPEG_H
