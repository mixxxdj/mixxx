#ifndef MIXXX_LEGACYAUDIOSOURCEADAPTER_H
#define MIXXX_LEGACYAUDIOSOURCEADAPTER_H


#include "sources/v1/legacyaudiosource.h"

#include "sources/audiosource.h"


namespace mixxx {

// forward declaration(s)
class AudioSource;

// Only required for SoundSourceCoreAudio.
class LegacyAudioSourceAdapter: public AudioSource {
  public:
    LegacyAudioSourceAdapter(
            AudioSource* pOwner,
            LegacyAudioSource* pImpl);

    void close() override {
        m_pOwner->close();
    }

  protected:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override {
        return tryOpenOn(*m_pOwner, mode, params);
    }

    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) override;

  private:
    AudioSource* m_pOwner;
    LegacyAudioSource* m_pImpl;
};

} // namespace mixxx


#endif // MIXXX_LEGACYAUDIOSOURCEADAPTER_H
