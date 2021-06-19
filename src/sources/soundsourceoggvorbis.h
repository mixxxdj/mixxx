#pragma once

#include "sources/soundsourceprovider.h"
#include "util/memory.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

QT_FORWARD_DECLARE_CLASS(QFile);

namespace mixxx {

class SoundSourceOggVorbis final : public SoundSource {
  public:
    explicit SoundSourceOggVorbis(const QUrl &url);
    ~SoundSourceOggVorbis() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams &params) override;

    static size_t ReadCallback(void *ptr, size_t size, size_t nmemb, void *datasource);
    static int SeekCallback(void *datasource, ogg_int64_t offset, int whence);
    static int CloseCallback(void *datasource);
    static long TellCallback(void *datasource);
    static ov_callbacks s_callbacks;

    std::unique_ptr<QFile> m_pFile;

    OggVorbis_File m_vf;

    SINT m_curFrameIndex;
};

class SoundSourceProviderOggVorbis : public SoundSourceProvider {
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

    SoundSourcePointer newSoundSource(const QUrl &url) override {
        return newSoundSourceFromUrl<SoundSourceOggVorbis>(url);
    }
};

} // namespace mixxx
