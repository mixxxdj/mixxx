#ifndef MIXXX_LEGACYAUDIOSOURCEADAPTER_H
#define MIXXX_LEGACYAUDIOSOURCEADAPTER_H


#include "sources/v1/legacyaudiosource.h"

#include "sources/audiosource.h"


namespace mixxx {

// forward declaration(s)
class AudioSource;

// Only required for SoundSourceCoreAudio.
class LegacyAudioSourceAdapter: public virtual ISampleFrameSource {
  public:
    LegacyAudioSourceAdapter(
            AudioSource* pOwner,
            LegacyAudioSource* pImpl);

    ReadableSampleFrames readSampleFramesClamped(
            ReadMode readMode,
            WritableSampleFrames sampleFrames) override;

  private:
    AudioSource* m_pOwner;
    LegacyAudioSource* m_pImpl;
};

} // namespace mixxx


#endif // MIXXX_LEGACYAUDIOSOURCEADAPTER_H
