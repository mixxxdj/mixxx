#ifndef MIXXX_SOUNDSOURCEM4A_H
#define MIXXX_SOUNDSOURCEM4A_H

#include "sources/soundsource.h"
#include "sources/soundsourceprovider.h"

#include "sources/libfaadloader.h"

#include "util/readaheadsamplebuffer.h"

extern "C" {

#include <libavformat/avformat.h>

} // extern "C"

#include <vector>

namespace mixxx {

/// Decode M4A (AAC in MP4) files using FAAD2.
///
/// NOTE(2020-05-01): Decoding in version v2.9.1 of the library
/// is broken and any attempt to open files will fail.
///
/// https://github.com/mixxxdj/mixxx/pull/2738
/// https://github.com/knik0/faad2/commit/a8dc3f8ce67f4069cfa4d5cf0fcc2c6e8ef2c2aa
/// https://github.com/knik0/faad2/commit/920ec985a74c6f88fe507181df07a0cd7e51d519
class SoundSourceM4A : public SoundSource {
  public:
    explicit SoundSourceM4A(const QUrl& url);
    ~SoundSourceM4A() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    bool openDecoder();
    bool reopenDecoder();
    bool replaceDecoder(
            faad2::DecoderHandle hNewDecoder);
    void closeDecoder();

    bool isValidSampleBlockId(int sampleBlockId) const;

    void restartDecoding(int sampleBlockId);

    int seekAndReadSampleBlock(
            int sampleBlockId,
            uint8_t** ppBytes,
            uint32_t* pNumBytes);

    faad2::LibLoader* const m_pFaad;

    int m_frameSize;
    int m_maxSampleBlockId;

    AVStream* m_pAvStream;

    typedef std::vector<u_int8_t> InputBuffer;
    InputBuffer m_inputBuffer;
    InputBuffer::size_type m_inputBufferLength;
    InputBuffer::size_type m_inputBufferOffset;

    OpenParams m_openParams;

    faad2::DecoderHandle m_hDecoder;
    SINT m_numberOfPrefetchSampleBlocks;
    int m_curSampleBlockId;

    ReadAheadSampleBuffer m_sampleBuffer;

    SINT m_curFrameIndex;

    // Takes ownership of an input format context and ensures that
    // the corresponding AVFormatContext is closed, either explicitly
    // or implicitly by the destructor. The wrapper can only be
    // moved, copying is disabled.
    class InputAVFormatContextPtr final {
      public:
        explicit InputAVFormatContextPtr(AVFormatContext* pAvInputFormatContext = nullptr)
                : m_pAvInputFormatContext(pAvInputFormatContext) {
        }
        InputAVFormatContextPtr(const InputAVFormatContextPtr&) = delete;
        InputAVFormatContextPtr(InputAVFormatContextPtr&& that)
                : m_pAvInputFormatContext(that.m_pAvInputFormatContext) {
            that.m_pAvInputFormatContext = nullptr;
        }
        ~InputAVFormatContextPtr() {
            close();
        }

        void take(AVFormatContext** ppavInputFormatContext);

        void close();

        friend void swap(InputAVFormatContextPtr& lhs, InputAVFormatContextPtr& rhs) {
            std::swap(lhs.m_pAvInputFormatContext, rhs.m_pAvInputFormatContext);
        }

        InputAVFormatContextPtr& operator=(const InputAVFormatContextPtr&) = delete;
        InputAVFormatContextPtr& operator=(InputAVFormatContextPtr&& that) {
            swap(*this, that);
            return *this;
        }

        AVFormatContext* operator->() {
            return m_pAvInputFormatContext;
        }
        operator AVFormatContext*() {
            return m_pAvInputFormatContext;
        }

      private:
        AVFormatContext* m_pAvInputFormatContext;
    };
    InputAVFormatContextPtr m_pAvInputFormatContext;
};

class SoundSourceProviderM4A : public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEM4A_H
