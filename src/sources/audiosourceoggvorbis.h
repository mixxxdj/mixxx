#ifndef AUDIOSOURCEOGGVORBIS_H
#define AUDIOSOURCEOGGVORBIS_H

#include "sources/audiosource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

namespace Mixxx {

class AudioSourceOggVorbis: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceOggVorbis();

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

private:
    explicit AudioSourceOggVorbis(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize,
            bool readStereoSamples);

    OggVorbis_File m_vf;

    SINT m_curFrameIndex;
};

}

#endif
