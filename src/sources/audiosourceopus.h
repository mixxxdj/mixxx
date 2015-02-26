#ifndef AUDIOSOURCEOPUS_H
#define AUDIOSOURCEOPUS_H

#include "sources/audiosource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

namespace Mixxx {

class AudioSourceOpus: public AudioSource {
public:
    static const size_type kFrameRate;

    static AudioSourcePointer create(QUrl url);

    ~AudioSourceOpus();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    size_type readSampleFramesStereo(size_type numberOfFrames,
            CSAMPLE* sampleBuffer, size_type sampleBufferSize) /*override*/;

private:
    explicit AudioSourceOpus(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    OggOpusFile *m_pOggOpusFile;

    diff_type m_curFrameIndex;
};

}
;

#endif
