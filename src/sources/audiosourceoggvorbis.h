#ifndef AUDIOSOURCEOGGVORBIS_H
#define AUDIOSOURCEOGGVORBIS_H

#include "sources/audiosource.h"
#include "util/defs.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

namespace Mixxx
{

class AudioSourceOggVorbis: public AudioSource {
public:
    static AudioSourcePointer open(QString fileName);

    ~AudioSourceOggVorbis();

    diff_type seekFrame(diff_type frameIndex) /*override*/;

    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;
    size_type readStereoFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

private:
    AudioSourceOggVorbis();

    Result postConstruct(QString fileName);
    void preDestroy();

    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer, bool readStereoSamples);

    OggVorbis_File m_vf;
};

}

#endif
