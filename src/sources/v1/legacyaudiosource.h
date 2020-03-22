#ifndef MIXXX_LEGACYAUDIOSOURCE_H
#define MIXXX_LEGACYAUDIOSOURCE_H

#include "util/types.h"

namespace mixxx {

// The legacy API that has been deprecated.
// Only required for SoundSourceCoreAudio.
class LegacyAudioSource {
  public:
    virtual ~LegacyAudioSource() = default;

    // Adjusts the current frame seek index:
    // - Precondition: isValidFrameIndex(frameIndex) == true
    // - The seek position in seconds is frameIndex / sampleRate()
    // Returns the actual current frame index which may differ from the
    // requested index if the source does not support accurate seeking.
    virtual SINT seekSampleFrame(SINT frameIndex) = 0;

    // Fills the buffer with samples from each channel starting
    // at the current frame seek position.
    //
    // The implicit minimum required capacity of the sampleBuffer is
    //     sampleBufferSize = frames2samples(numberOfFrames)
    // Samples in the sampleBuffer are stored as consecutive sample
    // frames with samples from each channel interleaved.
    //
    // Returns the actual number of frames that have been read which
    // might be lower than the requested number of frames when the end
    // of the audio stream has been reached. The current frame seek
    // position is moved forward towards the next unread frame.
    virtual SINT readSampleFrames(
            SINT numberOfFrames,
            CSAMPLE* sampleBuffer) = 0;
};

} // namespace mixxx

#endif // MIXXX_LEGACYAUDIOSOURCE_H
