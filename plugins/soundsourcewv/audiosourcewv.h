#ifndef AUDIOSOURCEWV_H
#define AUDIOSOURCEWV_H

#include "sources/audiosource.h"

#include "wavpack/wavpack.h"

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

namespace Mixxx {

class AudioSourceWV: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceWV();

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    explicit AudioSourceWV(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    WavpackContext* m_wpc;

    CSAMPLE m_sampleScale;
};

}  // namespace Mixxx

#endif
