#ifndef AUDIOSOURCEMODPLUG_H
#define AUDIOSOURCEMODPLUG_H

#include "sources/audiosource.h"

namespace ModPlug {
#include <libmodplug/modplug.h>
}

#include <vector>

namespace Mixxx {

// Class for reading tracker files using libmodplug.
// The whole file is decoded at once and saved
// in RAM to allow seeking and smooth operation in Mixxx.
class AudioSourceModPlug: public AudioSource {
public:
    static const SINT kChannelCount = 2; // always stereo
    static const SINT kFrameRate = 44100; // always 44.1kHz

    // apply settings for decoding
    static void configure(unsigned int bufferSizeLimit,
            const ModPlug::ModPlug_Settings &settings);

    static AudioSourcePointer create(QUrl url);

    ~AudioSourceModPlug();

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    static unsigned int s_bufferSizeLimit; // max track buffer length (bytes)

    explicit AudioSourceModPlug(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    ModPlug::ModPlugFile *m_pModFile; // modplug file descriptor
    SINT m_fileLength; // length of file in samples
    SINT m_seekPos; // current read position
    QByteArray m_fileBuf; // original module file data
    std::vector<SAMPLE> m_sampleBuf; // 16bit stereo samples, 44.1kHz
};

}

#endif
