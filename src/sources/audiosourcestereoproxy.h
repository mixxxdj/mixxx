#ifndef MIXXX_AUDIOSOURCESTEREOPROXY_H
#define MIXXX_AUDIOSOURCESTEREOPROXY_H


#include "sources/audiosource.h"


namespace mixxx {

class AudioSourceStereoProxy: public AudioSource {
public:
    static AudioSourcePointer create(
            AudioSourcePointer pAudioSource,
            SINT maxReadableFrames) {
        return std::make_shared<AudioSourceStereoProxy>(
                pAudioSource,
                maxReadableFrames);
    }

    AudioSourceStereoProxy(
            AudioSourcePointer pAudioSource,
            SINT maxReadableFrames);
    AudioSourceStereoProxy(
            AudioSourcePointer pAudioSource,
            SampleBuffer::WritableSlice tempSampleBufferSlice);

    ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames writableSampleFrames) override;

private:
    AudioSourcePointer m_pAudioSource;
    SampleBuffer m_tempSampleBuffer;
    SampleBuffer::WritableSlice m_tempOutputBuffer;
};

} // namespace mixxx


#endif // MIXXX_AUDIOSOURCESTEREOPROXY_H
