#ifndef MIXXX_SOUNDSOURCESNDFILE_H
#define MIXXX_SOUNDSOURCESNDFILE_H

#include "sources/soundsource.h"

#ifdef Q_OS_WIN
//Enable unicode in libsndfile on Windows
//(sf_open uses UTF-8 otherwise)
#include <windows.h>
#define ENABLE_SNDFILE_WINDOWS_PROTOTYPES 1
#endif
#include <sndfile.h>

namespace Mixxx {

class SoundSourceSndFile: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceSndFile(QUrl url);
    ~SoundSourceSndFile();

    Result open() /*override*/;
    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;

private:
    SNDFILE* m_pSndFile;
    SF_INFO m_sfInfo;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCESNDFILE_H
