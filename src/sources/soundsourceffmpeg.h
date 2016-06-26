#ifndef MIXXX_SOUNDSOURCEFFMPEG_H
#define MIXXX_SOUNDSOURCEFFMPEG_H

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#ifndef __FFMPEGOLDAPI__
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#endif

// Compatibility
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>

} // extern "C"

#include <QVector>

#include "sources/soundsourceprovider.h"

#include "util/memory.h" // std::unique_ptr<> + std::make_unique()

// forward declaration
class EncoderFfmpegResample;

namespace mixxx {

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
    explicit SoundSourceFFmpeg(const QUrl& url);
    ~SoundSourceFFmpeg() override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames, CSAMPLE* sampleBuffer) override;

  private:
    OpenResult tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    bool readFramesToCache(unsigned int count, SINT offset);
    bool getBytesFromCache(CSAMPLE* buffer, SINT offset, SINT size);
    SINT getSizeofCache();
    void clearCache();

    unsigned int read(unsigned long size, SAMPLE*);

    AVFormatContext *m_pFormatCtx;
    int m_iAudioStream;
    AVCodecContext *m_pCodecCtx;
    AVCodec *m_pCodec;

    std::unique_ptr<EncoderFfmpegResample> m_pResample;

    SINT m_currentMixxxFrameIndex;

    bool m_bIsSeeked;

    SINT m_lCacheFramePos;
    SINT m_lCacheStartFrame;
    SINT m_lCacheEndFrame;
    SINT m_lCacheLastPos;
    QVector<struct ffmpegCacheObject  *> m_SCache;
    QVector<struct ffmpegLocationObject  *> m_SJumpPoints;
    SINT m_lLastStoredPos;
    SINT m_lStoreCount;
    SINT m_lStoredSeekPoint;
    struct ffmpegLocationObject *m_SStoredJumpPoint;
};

class SoundSourceProviderFFmpeg: public SoundSourceProvider {
  public:
    SoundSourceProviderFFmpeg();

    QString getName() const override {
        return "FFmpeg";
    }

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override {
        Q_UNUSED(supportedFileExtension);
        // FFmpeg will only be used as the last resort
        return SoundSourceProviderPriority::LOWEST;
    }

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return SoundSourcePointer(new SoundSourceFFmpeg(url));
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEFFMPEG_H
