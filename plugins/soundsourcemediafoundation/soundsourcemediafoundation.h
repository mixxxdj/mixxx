#ifndef MIXXX_SOUNDSOURCEMEDIAFOUNDATION_H
#define MIXXX_SOUNDSOURCEMEDIAFOUNDATION_H


#include <mfidl.h>
#include <mfreadwrite.h>

#include "sources/soundsourceplugin.h"
#include "util/singularsamplebuffer.h"


namespace mixxx {

class StreamUnitConverter final {
    const static SINT kStreamUnitsPerSecond = 1000 * 1000 * 10; // frame length = 100 ns

  public:
    StreamUnitConverter()
        : m_streamUnitsPerFrame(0) {
    }
    explicit StreamUnitConverter(SINT frameRate)
        : m_streamUnitsPerFrame(double(kStreamUnitsPerSecond) / double(frameRate)),
          m_toFrameIndexBias(kStreamUnitsPerSecond / frameRate / 2) {
        // The stream units should actually be much shorter
        // than the frames to minimize jitter. Even a frame
        // at 192 kHz has a length of about 5000 ns >> 100 ns.
        DEBUG_ASSERT(m_streamUnitsPerFrame >= 50);
        DEBUG_ASSERT(m_toFrameIndexBias > 0);
    }

    LONGLONG fromFrameIndex(SINT frameIndex) const {
        // Used for seeking, so we need to round down to hit the
        // corresponding stream unit where the given stream unit
        // starts
        return floor((frameIndex - AudioSource::getMinFrameIndex()) * m_streamUnitsPerFrame);
    }

    SINT toFrameIndex(LONGLONG streamPos) const {
        // NOTE(uklotzde): Add m_toFrameIndexBias to account for rounding errors
        return AudioSource::getMinFrameIndex() +
                static_cast<SINT>(floor((streamPos + m_toFrameIndexBias) / m_streamUnitsPerFrame));
    }

  private:
    double m_streamUnitsPerFrame;
    SINT m_toFrameIndexBias;
};

class SoundSourceMediaFoundation : public mixxx::SoundSourcePlugin {
public:
    explicit SoundSourceMediaFoundation(const QUrl& url);
    ~SoundSourceMediaFoundation() override;

    void close() override;

    SINT seekSampleFrame(SINT frameIndex) override;

    SINT readSampleFrames(SINT numberOfFrames, CSAMPLE* sampleBuffer) override;

private:
    OpenResult tryOpen(const mixxx::AudioSourceConfig& audioSrcCfg) override;

    bool configureAudioStream(const mixxx::AudioSourceConfig& audioSrcCfg);
    bool readProperties();

    HRESULT m_hrCoInitialize;
    HRESULT m_hrMFStartup;

    IMFSourceReader* m_pSourceReader;

    StreamUnitConverter m_streamUnitConverter;

    SINT m_currentFrameIndex;

    SingularSampleBuffer m_sampleBuffer;
};

class SoundSourceProviderMediaFoundation: public SoundSourceProvider {
public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx


extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider();

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(mixxx::SoundSourceProvider*);


#endif // MIXXX_SOUNDSOURCEMEDIAFOUNDATION_H
