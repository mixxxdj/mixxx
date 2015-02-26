#ifndef AUDIOSOURCEOPUS_H
#define AUDIOSOURCEOPUS_H

#include "sources/audiosource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

namespace Mixxx {

class AudioSourceOpus: public AudioSource {
public:
    static const SINT kFrameRate;

    static AudioSourcePointer create(QUrl url);

    ~AudioSourceOpus();

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

private:
    explicit AudioSourceOpus(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    OggOpusFile *m_pOggOpusFile;

    SINT m_curFrameIndex;
};

}
;

#endif
