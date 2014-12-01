#ifndef SOUNDSOURCEOPUS_H
#define SOUNDSOURCEOPUS_H

#include "soundsource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

class SoundSourceOpus: public Mixxx::SoundSource {
    typedef SoundSource Super;

public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOpus(QString qFilename);
    ~SoundSourceOpus();

    Result parseHeader() /*override*/;
    QImage parseCoverArt() /*override*/;

    Result open() /*override*/;

    diff_type seekFrame(diff_type frameIndex);
    size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;
    size_type readStereoFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) /*override*/;

private:
    void close();

    OggOpusFile *m_pOggOpusFile;
};

#endif
