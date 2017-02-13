#ifndef MIXXX_SOUNDSOURCEMODPLUG_H
#define MIXXX_SOUNDSOURCEMODPLUG_H

#include "sources/soundsourceprovider.h"

namespace ModPlug {
#include <libmodplug/modplug.h>
}

#include <vector>

namespace mixxx {

// Class for reading tracker files using libmodplug.
// The whole file is decoded at once and saved
// in RAM to allow seeking and smooth operation in Mixxx.
class SoundSourceModPlug: public mixxx::SoundSource {
public:
     static constexpr SINT kChannelCount = kChannelCountStereo;
     static constexpr SINT kSamplingRate = 44100;
     static constexpr SINT kBitsPerSample = 16;

    // apply settings for decoding
    static void configure(unsigned int bufferSizeLimit,
            const ModPlug::ModPlug_Settings &settings);

    explicit SoundSourceModPlug(const QUrl& url);
    ~SoundSourceModPlug() override;

    Result parseTrackMetadataAndCoverArt(
            TrackMetadata* pTrackMetadata,
            QImage* pCoverArt) const override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames,
            CSAMPLE* sampleBuffer) override;

private:
    OpenResult tryOpen(const AudioSourceConfig& audioSrcCfg) override;

    static unsigned int s_bufferSizeLimit; // max track buffer length (bytes)

    ModPlug::ModPlugFile *m_pModFile; // modplug file descriptor
    QByteArray m_fileBuf; // original module file data

    typedef std::vector<SAMPLE> SampleBuffer;
    SampleBuffer m_sampleBuf;

    SINT m_seekPos; // current read position
};

class SoundSourceProviderModPlug: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override {
        return newSoundSourceFromUrl<SoundSourceModPlug>(url);
    }
};

} // namespace mixxx

#endif // MIXXX_SOUNDSOURCEMODPLUG_H
