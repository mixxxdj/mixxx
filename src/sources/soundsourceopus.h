#ifndef MIXXX_SOUNDSOURCEOPUS_H
#define MIXXX_SOUNDSOURCEOPUS_H

#include "sources/soundsourceprovider.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <opus/opusfile.h>

namespace Mixxx {

class SoundSourceOpus: public Mixxx::SoundSource {
public:
    static const SINT kSamplingRate;

    explicit SoundSourceOpus(QUrl url);
    ~SoundSourceOpus();

    Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) override;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    OggOpusFile *m_pOggOpusFile;

    SINT m_curFrameIndex;
};

class SoundSourceProviderOpus: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return SoundSourcePointer(new SoundSourceOpus(url));
    }
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEOPUS_H
