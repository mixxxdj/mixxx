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

        AVFormatContext* operator->() { return m_pClosableInputFormatContext; }
        operator AVFormatContext*() { return m_pClosableInputFormatContext; }

    private:
        AVFormatContext* m_pClosableInputFormatContext;
    };
    ClosableInputAVFormatContextPtr m_pInputFormatContext;

    static OpenResult openAudioStream(AVStream* pAudioStream);

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

        AVStream* operator->() { return m_pClosableStream; }
        operator AVStream*() { return m_pClosableStream; }

    private:
        AVStream* m_pClosableStream;
    };
    ClosableAVStreamPtr m_pAudioStream;

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
        return newSoundSourceFromUrl<SoundSourceFFmpeg>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEFFMPEG_H
