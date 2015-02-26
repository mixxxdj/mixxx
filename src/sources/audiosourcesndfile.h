#ifndef AUDIOSOURCESNDFILE_H
#define AUDIOSOURCESNDFILE_H

#include "sources/audiosource.h"

#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

namespace Mixxx {

class AudioSourceSndFile: public AudioSource {
public:
    static AudioSourcePointer create(QUrl url);

    ~AudioSourceSndFile();

    diff_type seekSampleFrame(diff_type frameIndex) /*override*/;

    size_type readSampleFrames(size_type numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    explicit AudioSourceSndFile(QUrl url);

    Result postConstruct() /*override*/;

    void preDestroy();

    SNDFILE* m_pSndFile;
    SF_INFO m_sfInfo;
};

}

#endif
