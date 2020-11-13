#pragma once

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

#include "sources/soundsourceprovider.h"
#include "util/samplebuffer.h"

namespace mixxx {

class SoundSourceOpus final : public SoundSource {
  public:
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
    static const QString kDisplayName;
    static const QStringList kSupportedFileExtensions;

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileExtensions() const override {
        return kSupportedFileExtensions;
    }

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceOpus>(url);
    }
};

} // namespace mixxx
