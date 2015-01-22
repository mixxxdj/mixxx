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

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer) /*override*/;

private:
    explicit AudioSourceWV(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    WavpackContext* m_wpc;

    sample_type m_sampleScale;
};

}  // namespace Mixxx

#endif
