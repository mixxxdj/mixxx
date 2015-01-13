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
    static AudioSourcePointer create(QString fileName);

    ~AudioSourceOggVorbis();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames, sample_type* sampleBuffer) /*override*/;
    size_type readSampleFramesStereo(size_type numberOfFrames, sample_type* sampleBuffer, size_type sampleBufferSize) /*override*/;

private:
    AudioSourceOggVorbis();

    Result open(QString fileName);

    void close();

    inline diff_type getCurrentFrameIndex() {
        return ov_pcm_tell(&m_vf);
    }

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize,
            bool readStereoSamples);

    OggVorbis_File m_vf;
};

}

#endif
