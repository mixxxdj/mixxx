#ifndef AUDIOSOURCEWV_H
#define AUDIOSOURCEWV_H

#include "sources/audiosource.h"
#include "util/defs.h"

#include "wavpack/wavpack.h"

#ifdef Q_OS_WIN
#define MY_EXPORT __declspec(dllexport)
#else
#define MY_EXPORT
#endif

namespace Mixxx {

class AudioSourceWV: public AudioSource {
    typedef AudioSource Super;

public:
    static AudioSourcePointer open(QString fileName);

    ~AudioSourceWV();

    diff_type seekFrame(diff_type frameIndex) /*override*/;

    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

    void close() throw() /* override*/;

private:
    AudioSourceWV();

    Result postConstruct(QString fileName);

    WavpackContext* m_wpc;

    sample_type m_sampleScale;
};

}  // namespace Mixxx

#endif
