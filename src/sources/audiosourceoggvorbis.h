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

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer) /*override*/;
    size_type readSampleFramesStereo(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize) /*override*/;

private:
    explicit AudioSourceOggVorbis(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer, size_type sampleBufferSize,
            bool readStereoSamples);

    OggVorbis_File m_vf;

    diff_type m_curFrameIndex;
};

}

#endif
