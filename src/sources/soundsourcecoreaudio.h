#pragma once

#include <vector>

#include <AudioToolbox/AudioToolbox.h>
//In our tree at lib/apple/
#include "CAStreamBasicDescription.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioFormat.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreServices/CoreServices.h>
#else
#include "AudioFile.h"
#include "AudioFormat.h"
#include "CoreAudioTypes.h"
#endif

#include "sources/soundsourceprovider.h"

#include "sources/v1/legacyaudiosourceadapter.h"

namespace mixxx {

class SoundSourceCoreAudio
        : public SoundSource,
          public virtual /*implements*/ LegacyAudioSource,
          public LegacyAudioSourceAdapter {
  public:
    explicit SoundSourceCoreAudio(QUrl url);
    ~SoundSourceCoreAudio() override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override;

    bool m_bFileIsMp3;
    ExtAudioFileRef m_audioFile;
    CAStreamBasicDescription m_inputFormat;
    CAStreamBasicDescription m_outputFormat;
    SINT m_leadingFrames;
    SINT m_seekPrefetchFrames;
    std::vector<CSAMPLE> m_seekPrefetchBuffer;
};

class SoundSourceProviderCoreAudio : public SoundSourceProvider {
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
        return newSoundSourceFromUrl<SoundSourceCoreAudio>(url);
    }
};

} // namespace mixxx
