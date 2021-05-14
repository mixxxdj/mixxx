#pragma once

#include "sources/v1/legacyaudiosource.h"

#include "sources/audiosource.h"

namespace mixxx {

// Only required for SoundSourceCoreAudio.
class LegacyAudioSourceAdapter : public virtual /*implements*/ IAudioSourceReader {
  public:
    LegacyAudioSourceAdapter(
            AudioSource* pOwner,
            LegacyAudioSource* pImpl);

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& sampleFrames) override;

  private:
    AudioSource* m_pOwner;
    LegacyAudioSource* m_pImpl;
};

} // namespace mixxx
