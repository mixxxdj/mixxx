#ifndef MIXXX_SOUNDSOURCEOPUS_H
#define MIXXX_SOUNDSOURCEOPUS_H

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

#include "sources/soundsourceprovider.h"
#include "util/samplebuffer.h"

namespace mixxx {

class SoundSourceOpus final : public SoundSource {
  public:
    // According to the API documentation of op_pcm_seek():
    // "...decoding after seeking may not return exactly the same
    // values as would be obtained by decoding the stream straight
    // through. However, such differences are expected to be smaller
    // than the loss introduced by Opus's lossy compression."
    // This implementation internally uses prefetching to compensate
    // those differences, although not completely. The following
    // constant indicates the maximum expected difference for
    // testing purposes.
    static const CSAMPLE kMaxDecodingError;

    explicit SoundSourceOpus(const QUrl& url);
    ~SoundSourceOpus() override;

    std::pair<ImportResult, QDateTime> importTrackMetadataAndCoverImage(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    OggOpusFile* m_pOggOpusFile;

    SampleBuffer m_prefetchSampleBuffer;

    SINT m_curFrameIndex;
};

class SoundSourceProviderOpus : public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceOpus>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEOPUS_H
