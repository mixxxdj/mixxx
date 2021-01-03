#pragma once

#include "sources/audiosourceproxy.h"

namespace mixxx {

class AudioSourceStereoProxy : public AudioSourceProxy {
  public:
    static AudioSourcePointer create(
            AudioSourcePointer pAudioSource,
            SINT maxReadableFrames) {
        return std::make_shared<AudioSourceStereoProxy>(
                pAudioSource,
                maxReadableFrames);
    }

    // Create an instance with its own temporary buffer
    AudioSourceStereoProxy(
            AudioSourcePointer pAudioSource,
            SINT maxReadableFrames);
    // Create an instance that borrows a writable slice of a
    // temporary buffer owned by the caller
    AudioSourceStereoProxy(
            AudioSourcePointer pAudioSource,
            SampleBuffer::WritableSlice tempWritableSlice);
    ~AudioSourceStereoProxy() override = default;

  protected:
    ReadableSampleFrames readSampleFramesClamped(
            const WritableSampleFrames& writableSampleFrames) override;

  private:
    SampleBuffer m_tempSampleBuffer;
    SampleBuffer::WritableSlice m_tempWritableSlice;
};

} // namespace mixxx
