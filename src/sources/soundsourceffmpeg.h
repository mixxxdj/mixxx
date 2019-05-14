#ifndef MIXXX_SOUNDSOURCEFFMPEG_H
#define MIXXX_SOUNDSOURCEFFMPEG_H

extern "C" {

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

#define AVSTREAM_FROM_API_VERSION_3_1 \
    (LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(57, 48, 0))

// forward declaration
class EncoderFfmpegResample;

namespace {

// Because 3.1 changed API how to access thigs in AVStream
// we'll separate this logic in own wrapper class
class AVStreamWrapper {
  public:
    virtual AVMediaType getMediaTypeOfStream(AVStream* pStream) = 0;
    virtual AVCodec* findDecoderForStream(AVStream* pStream) = 0;
    virtual SINT getChannelCountOfStream(AVStream* pStream) = 0;
    virtual SINT getSampleRateOfStream(AVStream* pStream) = 0;
    virtual AVSampleFormat getSampleFormatOfStream(AVStream* pStream) = 0;
};

// Implement classes for version before 3.1 and after that
#if AVSTREAM_FROM_API_VERSION_3_1
// This is after version 3.1
class AVStreamWrapperImpl : public AVStreamWrapper {
  public:
    AVMediaType getMediaTypeOfStream(AVStream* pStream) {
        return pStream->codecpar->codec_type;
    }

    AVCodec* findDecoderForStream(AVStream* pStream) {
        return avcodec_find_decoder(pStream->codecpar->codec_id);
    }

    SINT getChannelCountOfStream(AVStream* pStream) {
        return pStream->codecpar->channels;
    }

    SINT getSampleRateOfStream(AVStream* pStream) {
        return pStream->codecpar->sample_rate;
    }

    AVSampleFormat getSampleFormatOfStream(AVStream* pStream) {
        return (AVSampleFormat)pStream->codecpar->format;
    }
};
#else
class AVStreamWrapperImpl : public AVStreamWrapper {
  public:
    AVMediaType getMediaTypeOfStream(AVStream* pStream) {
        return pStream->codec->codec_type;
    }

    AVCodec* findDecoderForStream(AVStream* pStream) {
        return avcodec_find_decoder(pStream->codec->codec_id);
    }

    SINT getChannelCountOfStream(AVStream* pStream) {
        return pStream->codec->channels;
    }

    SINT getSampleRateOfStream(AVStream* pStream) {
        return pStream->codec->sample_rate;
    }

    AVSampleFormat getSampleFormatOfStream(AVStream* pStream) {
        return pStream->codec->sample_fmt;
    }
};
#endif

//AVStreamWrapperImpl *m_pAVStreamWrapper = new AVStreamWrapperImpl();
AVStreamWrapperImpl m_pAVStreamWrapper;

} // namespace

namespace mixxx {

struct ffmpegLocationObject {
    SINT pos;
    SINT pts;
    SINT startFrame;
};

struct ffmpegCacheObject {
    SINT startFrame;
    SINT length;
    quint8* bytes;
};

class SoundSourceFFmpeg : public SoundSource {
  public:
    explicit SoundSourceFFmpeg(const QUrl& url);
    ~SoundSourceFFmpeg() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    bool readFramesToCache(unsigned int count, SINT offset);
    bool getBytesFromCache(CSAMPLE* buffer, SINT offset, SINT size);
    SINT getSizeofCache();
    void clearCache();

    unsigned int read(unsigned long size, SAMPLE*);

    static AVFormatContext* openInputFile(const QString& fileName);

    // Takes ownership of an input format context and ensures that
    // the corresponding AVFormatContext is closed, either explicitly
    // or implicitly by the destructor. The wrapper can only be
    // moved, copying is disabled.
    class ClosableInputAVFormatContextPtr final {
      public:
        explicit ClosableInputAVFormatContextPtr(AVFormatContext* pClosableInputFormatContext = nullptr)
                : m_pClosableInputFormatContext(pClosableInputFormatContext) {
        }
        explicit ClosableInputAVFormatContextPtr(const ClosableInputAVFormatContextPtr&) = delete;
        explicit ClosableInputAVFormatContextPtr(ClosableInputAVFormatContextPtr&& that)
                : m_pClosableInputFormatContext(that.m_pClosableInputFormatContext) {
            that.m_pClosableInputFormatContext = nullptr;
        }
        ~ClosableInputAVFormatContextPtr() {
            close();
        }

        void take(AVFormatContext** ppClosableInputFormatContext);

        void close();

        friend void swap(ClosableInputAVFormatContextPtr& lhs, ClosableInputAVFormatContextPtr& rhs) {
            std::swap(lhs.m_pClosableInputFormatContext, rhs.m_pClosableInputFormatContext);
        }

        ClosableInputAVFormatContextPtr& operator=(const ClosableInputAVFormatContextPtr&) = delete;
        ClosableInputAVFormatContextPtr& operator=(ClosableInputAVFormatContextPtr&& that) = delete;

        AVFormatContext* operator->() {
            return m_pClosableInputFormatContext;
        }
        operator AVFormatContext*() {
            return m_pClosableInputFormatContext;
        }

      private:
        AVFormatContext* m_pClosableInputFormatContext;
    };
    ClosableInputAVFormatContextPtr m_pInputFormatContext;

    static OpenResult openAudioStream(AVCodecContext* pCodecContext, AVCodec* pDecoder);

    // Takes ownership of an opened (audio) stream and ensures that
    // the corresponding AVStream is closed, either explicitly or
    // implicitly by the destructor. The wrapper can only be moved,
    // copying is disabled.
    class ClosableAVStreamPtr final {
      public:
        explicit ClosableAVStreamPtr(AVStream* pClosableStream = nullptr)
                : m_pClosableStream(pClosableStream) {
        }
        explicit ClosableAVStreamPtr(const ClosableAVStreamPtr&) = delete;
        explicit ClosableAVStreamPtr(ClosableAVStreamPtr&& that)
                : m_pClosableStream(that.m_pClosableStream) {
            that.m_pClosableStream = nullptr;
        }
        ~ClosableAVStreamPtr() {
            close();
        }

        void take(AVStream** ppClosableStream);
        void close();

        friend void swap(ClosableAVStreamPtr& lhs, ClosableAVStreamPtr& rhs) {
            std::swap(lhs.m_pClosableStream, rhs.m_pClosableStream);
        }

        ClosableAVStreamPtr& operator=(const ClosableAVStreamPtr&) = delete;
        ClosableAVStreamPtr& operator=(ClosableAVStreamPtr&& that) = delete;

        AVStream* operator->() {
            return m_pClosableStream;
        }
        operator AVStream*() {
            return m_pClosableStream;
        }

      private:
        AVStream* m_pClosableStream;
    };
    ClosableAVStreamPtr m_pAudioStream;

#if AVSTREAM_FROM_API_VERSION_3_1
    // Takes ownership of an opened (audio) codec context and ensures that
    // the corresponding AVCodecContext is closed, either explicitly or
    // implicitly by the destructor. The wrapper can only be moved,
    // copying is disabled.
    //
    // This is prior new API changes made in FFMmpeg 3.1
    // before that we can use AVStream->codec to access AVCodecContext
    class ClosableAVCodecContextPtr final {
      public:
        explicit ClosableAVCodecContextPtr(AVCodecContext* pClosableContext = nullptr)
                : m_pClosableContext(pClosableContext) {
        }
        explicit ClosableAVCodecContextPtr(const ClosableAVCodecContextPtr&) = delete;
        explicit ClosableAVCodecContextPtr(ClosableAVCodecContextPtr&& that)
                : m_pClosableContext(that.m_pClosableContext) {
            that.m_pClosableContext = nullptr;
        }
        ~ClosableAVCodecContextPtr() {
            close();
        }

        void take(AVCodecContext** ppClosableContext);
        void close();

        friend void swap(ClosableAVCodecContextPtr& lhs, ClosableAVCodecContextPtr& rhs) {
            std::swap(lhs.m_pClosableContext, rhs.m_pClosableContext);
        }

        ClosableAVCodecContextPtr& operator=(const ClosableAVCodecContextPtr&) = delete;
        ClosableAVCodecContextPtr& operator=(ClosableAVCodecContextPtr&& that) = delete;

        AVCodecContext* operator->() {
            return m_pClosableContext;
        }
        operator AVCodecContext*() {
            return m_pClosableContext;
        }

      private:
        AVCodecContext* m_pClosableContext;
    };
    ClosableAVCodecContextPtr m_pAudioContext;
#endif

    std::unique_ptr<EncoderFfmpegResample> m_pResample;

    SINT m_currentMixxxFrameIndex;

    bool m_bIsSeeked;

    SINT m_lCacheFramePos;
    SINT m_lCacheStartFrame;
    SINT m_lCacheEndFrame;
    SINT m_lCacheLastPos;
    QVector<struct ffmpegCacheObject*> m_SCache;
    QVector<struct ffmpegLocationObject*> m_SJumpPoints;
    SINT m_lLastStoredPos;
    SINT m_lStoreCount;
    SINT m_lStoredSeekPoint;
    struct ffmpegLocationObject* m_SStoredJumpPoint;
};

class SoundSourceProviderFFmpeg : public SoundSourceProvider {
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
        return newSoundSourceFromUrl<SoundSourceFFmpeg>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEFFMPEG_H
