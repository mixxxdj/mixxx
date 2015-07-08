#ifndef MIXXX_SOUNDSOURCEOGGVORBIS_H
#define MIXXX_SOUNDSOURCEOGGVORBIS_H

#include "sources/soundsource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

namespace Mixxx {

class SoundSourceOggVorbis: public SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    explicit SoundSourceOggVorbis(QUrl url);
    ~SoundSourceOggVorbis();

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize,
            bool readStereoSamples);

    OggVorbis_File m_vf;

    SINT m_curFrameIndex;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEOGGVORBIS_H
