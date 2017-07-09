#ifndef MIXXX_AUDIOSOURCESTEREOPROXY_H
#define MIXXX_AUDIOSOURCESTEREOPROXY_H


#include "sources/audiosource.h"


namespace mixxx {

class AudioSourceStereoProxy: public AudioSource {
public:
    static SINT calcTempBufferSize(
            const AudioSourcePointer& pAudioSource,
            SINT maxReadableFrames) {
        return pAudioSource->frames2samples(maxReadableFrames);
    }

    AudioSourceStereoProxy(
            AudioSourcePointer pAudioSource,
            SampleBuffer::WritableSlice tempSampleBuffer);

    IndexRange readOrSkipSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::WritableSlice* pOutputBuffer) override;

    IndexRange skipSampleFrames(
            IndexRange frameIndexRange) override {
        return m_pAudioSource->skipSampleFrames(frameIndexRange);
    }

private:
    AudioSourcePointer m_pAudioSource;
    SampleBuffer::WritableSlice m_tempSampleBuffer;
};

} // namespace mixxx


#endif // MIXXX_AUDIOSOURCESTEREOPROXY_H
