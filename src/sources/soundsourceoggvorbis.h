#ifndef MIXXX_SOUNDSOURCEOGGVORBIS_H
#define MIXXX_SOUNDSOURCEOGGVORBIS_H

#include "sources/soundsourceprovider.h"
#include "util/memory.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

class QFile;

namespace mixxx {

class SoundSourceOggVorbis final : public SoundSource {
  public:
    explicit SoundSourceOggVorbis(const QUrl &url);
    ~SoundSourceOggVorbis() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

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
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl &url) override {
        return newSoundSourceFromUrl<SoundSourceOggVorbis>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEOGGVORBIS_H
