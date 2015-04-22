#ifndef MIXXX_SOUNDSOURCEOPUS_H
#define MIXXX_SOUNDSOURCEOPUS_H

#include "sources/soundsource.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

namespace Mixxx {

class SoundSourceOpus: public Mixxx::SoundSource {
public:
    static QList<QString> supportedFileExtensions();

    static const SINT kFrameRate;

    explicit SoundSourceOpus(QUrl url);
    ~SoundSourceOpus();

    Result parseTrackMetadata(Mixxx::TrackMetadata* pMetadata) const /*override*/;

    void close() /*override*/;

    SINT seekSampleFrame(SINT frameIndex) /*override*/;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) /*override*/;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) /*override*/;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) /*override*/;

    OggOpusFile *m_pOggOpusFile;

    SINT m_curFrameIndex;
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEOPUS_H
