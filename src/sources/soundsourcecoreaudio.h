#ifndef SOUNDSOURCECOREAUDIO_H
#define SOUNDSOURCECOREAUDIO_H

#include "sources/soundsourceprovider.h"

#include "sources/v1/legacyaudiosourceadapter.h"

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

namespace mixxx {

class SoundSourceCoreAudio : public SoundSource, public virtual /*implements*/ LegacyAudioSource, public LegacyAudioSourceAdapter {
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
    SInt64 m_headerFrames;
};

class SoundSourceProviderCoreAudio : public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceCoreAudio>(url);
    }
};

} // namespace mixxx

#endif // SOUNDSOURCECOREAUDIO_H
