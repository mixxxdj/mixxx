#ifndef SOUNDSOURCESNDFILE_H
#define SOUNDSOURCESNDFILE_H

#include "soundsource.h"

#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

class SoundSourceSndFile: public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceSndFile(QString qFilename);
    ~SoundSourceSndFile();

    Result parseHeader() /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;

    diff_type seekFrame(diff_type frameIndex) /*override*/;
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    void close();

    SNDFILE* m_pSndFile;
    SF_INFO m_sfInfo;
};

#endif
