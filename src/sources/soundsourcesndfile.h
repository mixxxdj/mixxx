#pragma once

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
            const WritableSampleFrames& sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    SNDFILE* m_pSndFile;

    SINT m_curFrameIndex;
};

class SoundSourceProviderSndFile : public SoundSourceProvider {
  public:
    static const QString kDisplayName;

    SoundSourceProviderSndFile();

    QString getDisplayName() const override {
        return kDisplayName;
    }

    QStringList getSupportedFileExtensions() const override;

    SoundSourceProviderPriority getPriorityHint(
            const QString& supportedFileExtension) const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceSndFile>(url);
    }

  private:
    const QStringList m_supportedFileExtensions;
};

} // namespace mixxx
