#ifndef AUDIOSOURCESNDFILE_H
#define AUDIOSOURCESNDFILE_H

#include "sources/audiosource.h"
#include "util/defs.h"

#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

namespace Mixxx
{

class AudioSourceSndFile: public AudioSource {
    typedef AudioSource Super;

public:
    static AudioSourcePointer open(QString fileName);

    ~AudioSourceSndFile();

    diff_type seekFrame(diff_type frameIndex) /*override*/;

    size_type readFrameSamplesInterleaved(size_type frameCount, sample_type* sampleBuffer) /*override*/;

    void close() throw() /*override*/;

private:
    AudioSourceSndFile();

    Result postConstruct(QString fileName);

    SNDFILE* m_pSndFile;
    SF_INFO m_sfInfo;
};

}

#endif
