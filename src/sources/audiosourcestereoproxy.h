#ifndef MIXXX_AUDIOSOURCESTEREOPROXY_H
#define MIXXX_AUDIOSOURCESTEREOPROXY_H

#include "sources/audiosource.h"

namespace mixxx {

class AudioSourceStereoProxy : public AudioSource {
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

    void close() override {
        m_pAudioSource->close();
    }

  protected:
    OpenResult tryOpen(
            OpenMode mode,
            const OpenParams& params) override {
        return tryOpenOn(*m_pAudioSource, mode, params);
    }

    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames writableSampleFrames) override;

  private:
    AudioSourcePointer m_pAudioSource;
    SampleBuffer m_tempSampleBuffer;
    SampleBuffer::WritableSlice m_tempWritableSlice;
};

} // namespace mixxx

#endif // MIXXX_AUDIOSOURCESTEREOPROXY_H
