#ifndef AUDIOSOURCECOREAUDIO_H
#define AUDIOSOURCECOREAUDIO_H

#include "sources/audiosource.h"
#include "util/defs.h"

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

class AudioSourceCoreAudio : public AudioSource {
    typedef AudioSource Super;

public:
    static AudioSourcePointer open(QString fileName);

    ~AudioSourceCoreAudio();

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

    void close() throw() /*override*/;

private:
    AudioSourceCoreAudio();

    Result postConstruct(QString fileName);

    ExtAudioFileRef m_audioFile;
    CAStreamBasicDescription m_inputFormat;
    CAStreamBasicDescription m_outputFormat;
    SInt64 m_headerFrames;
};

}

#endif // ifndef AUDIOSOURCECOREAUDIO_H
