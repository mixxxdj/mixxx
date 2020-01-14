#ifndef MIXXX_SOUNDSOURCESNDFILE_H
#define MIXXX_SOUNDSOURCESNDFILE_H

#include "sources/soundsourceprovider.h"

#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

namespace mixxx {

class SoundSourceSndFile final : public SoundSource {
  public:
    explicit SoundSourceSndFile(const QUrl& url);
    ~SoundSourceSndFile() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    SNDFILE* m_pSndFile;

    SINT m_curFrameIndex;
};

class SoundSourceProviderSndFile : public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceSndFile>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCESNDFILE_H
