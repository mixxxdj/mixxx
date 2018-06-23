#ifndef MIXXX_SOUNDSOURCEMEDIAFOUNDATION_H
#define MIXXX_SOUNDSOURCEMEDIAFOUNDATION_H


#include <mfidl.h>
#include <mfreadwrite.h>

#include "sources/soundsourceplugin.h"
#include "util/readaheadsamplebuffer.h"


namespace mixxx {

class StreamUnitConverter final {
    const static SINT kStreamUnitsPerSecond = 1000 * 1000 * 10; // frame length = 100 ns

  public:
    StreamUnitConverter()
        : m_pAudioSource(nullptr),
          m_fromSampleFramesToStreamUnits(0),
          m_fromStreamUnitsToSampleFrames(0) {
    }
    explicit StreamUnitConverter(const AudioSource* pAudioSource)
        : m_pAudioSource(pAudioSource),
          m_fromSampleFramesToStreamUnits(double(kStreamUnitsPerSecond) / double(pAudioSource->sampleRate())),
          m_fromStreamUnitsToSampleFrames(double(pAudioSource->sampleRate()) / double(kStreamUnitsPerSecond)){
        // The stream units should actually be much shorter than
        // sample frames to minimize jitter and rounding. Even a
        // frame at 192 kHz has a length of about 5000 ns >> 100 ns.
        DEBUG_ASSERT(m_fromStreamUnitsToSampleFrames >= 50);
    }

    LONGLONG fromFrameIndex(SINT frameIndex) const {
        DEBUG_ASSERT(m_fromSampleFramesToStreamUnits > 0);
        // Used for seeking, so we need to round down to hit the
        // corresponding stream unit where the given stream unit
        // starts. The reader will skip samples until it reaches
        // the actual target position for reading.
        const SINT frameIndexOffset = frameIndex - m_pAudioSource->frameIndexMin();
        return static_cast<LONGLONG>(floor(frameIndexOffset * m_fromSampleFramesToStreamUnits));
    }

    SINT toFrameIndex(LONGLONG streamPos) const {
        DEBUG_ASSERT(m_fromStreamUnitsToSampleFrames > 0);
        // The stream reports positions in units of 100ns. We have
        // to round(!) this value to obtain the actual position in
        // sample frames.
        const SINT frameIndexOffset = static_cast<SINT>(round(streamPos * m_fromStreamUnitsToSampleFrames));
        return m_pAudioSource->frameIndexMin() + frameIndexOffset;
    }

  private:
    const AudioSource* m_pAudioSource;
    double m_fromSampleFramesToStreamUnits;
    double m_fromStreamUnitsToSampleFrames;
};

class SoundSourceMediaFoundation: public mixxx::SoundSourcePlugin {
  public:
    explicit SoundSourceMediaFoundation(const QUrl& url);
    ~SoundSourceMediaFoundation() override;

    void close() override;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    OpenResult tryOpen(
            OpenMode mode,
            const mixxx::AudioSource::OpenParams& params) override;

    bool configureAudioStream(const mixxx::AudioSource::OpenParams& params);
    bool readProperties();

    void seekSampleFrame(SINT frameIndex);

    HRESULT m_hrCoInitialize;
    HRESULT m_hrMFStartup;

    IMFSourceReader* m_pSourceReader;

    StreamUnitConverter m_streamUnitConverter;

    SINT m_currentFrameIndex;

    ReadAheadSampleBuffer m_sampleBuffer;
};

class SoundSourceProviderMediaFoundation: public SoundSourceProvider {
  public:
    QString getName() const override;

    QStringList getSupportedFileExtensions() const override;

    SoundSourcePointer newSoundSource(const QUrl& url) override;
};

} // namespace mixxx


extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
mixxx::SoundSourceProvider* Mixxx_SoundSourcePluginAPI_createSoundSourceProvider(int logLevel, int logFlushLevel);

extern "C" MIXXX_SOUNDSOURCEPLUGINAPI_EXPORT
void Mixxx_SoundSourcePluginAPI_destroySoundSourceProvider(mixxx::SoundSourceProvider*);


#endif // MIXXX_SOUNDSOURCEMEDIAFOUNDATION_H
