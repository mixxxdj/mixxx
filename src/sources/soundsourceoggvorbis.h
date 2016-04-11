#ifndef MIXXX_SOUNDSOURCEOGGVORBIS_H
#define MIXXX_SOUNDSOURCEOGGVORBIS_H

#include "sources/soundsourceprovider.h"

#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>

class QFile;

namespace Mixxx {

class SoundSourceOggVorbis: public SoundSource {
public:
    explicit SoundSourceOggVorbis(QUrl url);
    ~SoundSourceOggVorbis();

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;
    SINT readSampleFramesStereo(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize) override;

private:
    Result tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer, SINT sampleBufferSize,
            bool readStereoSamples);

    static size_t ReadCallback(void *ptr, size_t size, size_t nmemb,
            void *datasource);
    static int SeekCallback(void *datasource, ogg_int64_t offset, int whence);
    static int CloseCallback(void *datasource);
    static long TellCallback(void *datasource);
    static ov_callbacks s_callbacks;

    OggVorbis_File m_vf;

    SINT m_curFrameIndex;
    QFile* m_pFile;
};

class SoundSourceProviderOggVorbis: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return SoundSourcePointer(new SoundSourceOggVorbis(url));
    }
};

} // namespace Mixxx

#endif // MIXXX_SOUNDSOURCEOGGVORBIS_H
