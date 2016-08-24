#ifndef SOUNDSOURCECOREAUDIO_H
#define SOUNDSOURCECOREAUDIO_H

#include "sources/soundsourceprovider.h"

#include <AudioToolbox/AudioToolbox.h>
//In our tree at lib/apple/
#include "CAStreamBasicDescription.h"

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <CoreServices/CoreServices.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioFormat.h>
#else
#include "CoreAudioTypes.h"
#include "AudioFile.h"
#include "AudioFormat.h"
#endif

namespace mixxx {

class SoundSourceCoreAudio : public mixxx::SoundSource {
public:
    explicit SoundSourceCoreAudio(QUrl url);
    ~SoundSourceCoreAudio() override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;

private:
    OpenResult tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    bool m_bFileIsMp3;
    ExtAudioFileRef m_audioFile;
    CAStreamBasicDescription m_inputFormat;
    CAStreamBasicDescription m_outputFormat;
    SInt64 m_headerFrames;
};

class SoundSourceProviderCoreAudio: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceCoreAudio>(url);
    }
};

}  // namespace mixxx

#endif // SOUNDSOURCECOREAUDIO_H
