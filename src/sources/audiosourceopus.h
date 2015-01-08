#ifndef AUDIOSOURCEOPUS_H
#define AUDIOSOURCEOPUS_H

#include "sources/audiosource.h"
#include "util/defs.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

namespace Mixxx
{

class AudioSourceOpus: public AudioSource {
public:
    // All Opus audio is encoded at 48 kHz
    static const size_type kFrameRate = 48000;

    static AudioSourcePointer open(QString fileName);

    ~AudioSourceOpus();

    diff_type seekFrame(diff_type frameIndex) /*override*/;

    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;
    size_type readStereoFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

    void close() throw() /*override*/;

private:
    AudioSourceOpus();

    Result postConstruct(QString fileName);

    OggOpusFile *m_pOggOpusFile;
};

};

#endif
