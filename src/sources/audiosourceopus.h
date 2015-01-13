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

    static AudioSourcePointer create(QString fileName);

    ~AudioSourceOpus();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames, sample_type* sampleBuffer) /*override*/;
    size_type readSampleFramesStereo(size_type numberOfFrames, sample_type* sampleBuffer, size_type sampleBufferSize) /*override*/;

private:
    AudioSourceOpus();

    Result open(QString fileName);

    void close();

    inline diff_type getCurrentFrameIndex() const {
        DEBUG_ASSERT(NULL != m_pOggOpusFile);
        return op_pcm_tell(m_pOggOpusFile);
    }

    OggOpusFile *m_pOggOpusFile;
};

};

#endif
