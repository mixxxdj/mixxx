#ifndef AUDIOSOURCECOREAUDIO_H
#define AUDIOSOURCECOREAUDIO_H

#include "sources/audiosource.h"

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

namespace Mixxx {

class AudioSourceCoreAudio: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceCoreAudio();

    SINT seekSampleFrame(SINT frameIndex) /*override*/;
    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    explicit AudioSourceCoreAudio(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    ExtAudioFileRef m_audioFile;
    CAStreamBasicDescription m_inputFormat;
    CAStreamBasicDescription m_outputFormat;
    SInt64 m_headerFrames;
};

}

#endif // ifndef AUDIOSOURCECOREAUDIO_H
