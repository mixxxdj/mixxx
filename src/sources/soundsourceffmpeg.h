#pragma once

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

} // extern "C"

#include "sources/readaheadframebuffer.h"
#include "sources/soundsourceprovider.h"

namespace mixxx {

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

    bool initResampling(
            audio::ChannelCount* pResampledChannelCount,
            audio::SampleRate* pResampledSampleRate);
    const CSAMPLE* resampleDecodedAVFrame();

    // Seek to the requested start index (if needed) or return false
    // upon seek errors.
    bool adjustCurrentPosition(
            SINT startIndex);

    bool consumeNextAVPacket(
            AVPacket* pavPacket,
            AVPacket** ppavNextPacket);

    // Takes ownership of an input format context and ensures that
    // the corresponding AVFormatContext is closed, either explicitly
    // or implicitly by the destructor. The wrapper can only be
    // moved, copying is disabled.
    class InputAVFormatContextPtr final {
      public:
        explicit InputAVFormatContextPtr(AVFormatContext* pavInputFormatContext = nullptr)
                : m_pavInputFormatContext(pavInputFormatContext) {
        }
        InputAVFormatContextPtr(const InputAVFormatContextPtr&) = delete;
        InputAVFormatContextPtr(InputAVFormatContextPtr&& that)
                : m_pavInputFormatContext(that.m_pavInputFormatContext) {
            that.m_pavInputFormatContext = nullptr;
        }
        ~InputAVFormatContextPtr() {
            close();
        }

        void take(AVFormatContext** ppavInputFormatContext);

        void close();

        friend void swap(InputAVFormatContextPtr& lhs, InputAVFormatContextPtr& rhs) {
            std::swap(lhs.m_pavInputFormatContext, rhs.m_pavInputFormatContext);
        }

        InputAVFormatContextPtr& operator=(const InputAVFormatContextPtr&) = delete;
        InputAVFormatContextPtr& operator=(InputAVFormatContextPtr&& that) {
            swap(*this, that);
            return *this;
        }

        AVFormatContext* operator->() {
            return m_pavInputFormatContext;
        }
        operator AVFormatContext*() {
            return m_pavInputFormatContext;
        }

      private:
        AVFormatContext* m_pavInputFormatContext;
    };
    InputAVFormatContextPtr m_pavInputFormatContext;

    AVStream* m_pavStream;

    // Takes ownership of an opened (audio) codec context and ensures that
    // the corresponding AVCodecContext is closed, either explicitly or
    // implicitly by the destructor. The wrapper can only be moved,
    // copying is disabled.
    class AVCodecContextPtr final {
      public:
        static AVCodecContextPtr alloc(const AVCodec* codec);

        explicit AVCodecContextPtr(AVCodecContext* pavCodecContext = nullptr)
                : m_pavCodecContext(pavCodecContext) {
        }
        AVCodecContextPtr(const AVCodecContextPtr&) = delete;
        AVCodecContextPtr(AVCodecContextPtr&& that)
                : m_pavCodecContext(that.m_pavCodecContext) {
            that.m_pavCodecContext = nullptr;
        }
        ~AVCodecContextPtr() {
            close();
        }

        void take(AVCodecContext** ppavCodecContext);
        void close();

        friend void swap(AVCodecContextPtr& lhs, AVCodecContextPtr& rhs) {
            std::swap(lhs.m_pavCodecContext, rhs.m_pavCodecContext);
        }

        AVCodecContextPtr& operator=(const AVCodecContextPtr&) = delete;
        AVCodecContextPtr& operator=(AVCodecContextPtr&& that) {
            swap(*this, that);
            return *this;
        }

        AVCodecContext* operator->() {
            return m_pavCodecContext;
        }
        operator AVCodecContext*() {
            return m_pavCodecContext;
        }

      private:
        AVCodecContext* m_pavCodecContext;
    };
    AVCodecContextPtr m_pavCodecContext;

    // Resampler
    class SwrContextPtr final {
      public:
        explicit SwrContextPtr(SwrContext* m_pSwrContext = nullptr)
                : m_pSwrContext(m_pSwrContext) {
        }
        SwrContextPtr(const SwrContextPtr&) = delete;
        SwrContextPtr(SwrContextPtr&& that)
                : m_pSwrContext(that.m_pSwrContext) {
            that.m_pSwrContext = nullptr;
        }
        ~SwrContextPtr() {
            close();
        }

        void take(SwrContext** ppSwrContext);

        void close();

        friend void swap(SwrContextPtr& lhs, SwrContextPtr& rhs) {
            std::swap(lhs.m_pSwrContext, rhs.m_pSwrContext);
        }

        SwrContextPtr& operator=(const SwrContextPtr&) = delete;
        SwrContextPtr& operator=(SwrContextPtr&& that) {
            swap(*this, that);
            return *this;
        }

        SwrContext* operator->() {
            return m_pSwrContext;
        }
        operator SwrContext*() {
            return m_pSwrContext;
        }

      private:
        SwrContext* m_pSwrContext;
    };
    SwrContextPtr m_pSwrContext;

    uint64_t m_avStreamChannelLayout;
    uint64_t m_avResampledChannelLayout;

    AVFrame* m_pavDecodedFrame;
    AVFrame* m_pavResampledFrame;

    FrameCount m_seekPrerollFrameCount;

    ReadAheadFrameBuffer m_frameBuffer;
};

class SoundSourceProviderFFmpeg : public SoundSourceProvider {
  public:
    static const QString kDisplayName;

    SoundSourceProviderFFmpeg();

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileExtensions() const override;

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceFFmpeg>(url);
    }
};

} // namespace mixxx
