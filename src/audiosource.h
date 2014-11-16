#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "util/types.h"

#include <QSharedPointer>

#include <cstddef> // size_t / diff_t

namespace Mixxx {

// Common interface and base class for audio sources.
//
// Both the number of channels and the sample rate must
// be constant and are not allowed to change over time.
//
// The length of audio data is measured in frames. A frame
// is a tuple of samples, one for each channel.
class AudioSource {
public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t diff_type;
    typedef CSAMPLE sample_type;

    static const size_type kChannelCountZero = 0;
    static const size_type kChannelCountMono = 1;
    static const size_type kChannelCountStereo = 2;
    static const size_type kChannelCountDefault = kChannelCountZero;

    static const size_type kSampleRateZero = 0;
    static const size_type kSampleRateDefault = kSampleRateZero;

    static const size_type kFrameCountZero = 0;
    static const size_type kFrameCountDefault = kFrameCountZero;

    static const sample_type kSampleValueZero;
    static const sample_type kSampleValuePeak;

    virtual ~AudioSource();

    // Returns the number of channels.
    //
    // The number of channels must be constant over the whole
    // time.
    size_type getChannelCount() const {
        return m_channelCount;
    }
    void setChannelCount(size_type channelCount);

    bool isChannelCountValid() const {
        return kChannelCountZero < getChannelCount();
    }
    bool isChannelCountMono() const {
        return kChannelCountMono == getChannelCount();
    }
    bool isChannelCountStereo() const {
        return kChannelCountStereo == getChannelCount();
    }

    // Returns the number of samples for each channel per second.
    //
    // The sample rate must be uniform among all channels and
    // equals the number of frames per second. The sample rate
    // must be constant over the whole time.
    size_type getSampleRate() const {
        return m_sampleRate;
    }
    void setSampleRate(size_type sampleRate);

    bool isSampleRateValid() const {
        return kSampleRateZero < getSampleRate();
    }

    // Returns the total number of frames.
    size_type getFrameCount() const {
        return m_frameCount;
    }
    void setFrameCount(size_type frameCount) {
        m_frameCount = frameCount;
    }

    bool isFrameCountEmpty() const {
        return kFrameCountZero >= getFrameCount();
    }

    // #frames -> #samples
    template<typename T>
    T frames2samples(T frameCount) const {
        Q_ASSERT(isChannelCountValid());
        return frameCount * getChannelCount();
    }

    // #samples -> #frames
    template<typename T>
    T samples2frames(T sampleCount) const {
        Q_ASSERT(isChannelCountValid());
        Q_ASSERT(0 == (sampleCount % getChannelCount()));
        return sampleCount / getChannelCount();
    }

    // Adjusts the current frame seek index:
    // - Index of first frame: frameIndex = 0
    // - Index of last frame: frameIndex = totalFrames() - 1
    // - The seek position in seconds is frameIndex / sampleRate()
    //
    // Returns the actual current frame index which may differ from the
    // requested index if the source does not support accurate seeking.
    virtual diff_type seekFrame(diff_type frameIndex) = 0;

    // Fills the buffer with samples from each channel starting
    // at the current frame seek position.
    //
    // The required size of the sampleBuffer is sampleCount =
    // frames2samples(frameCount). The samples in the sampleBuffer
    // are stored as consecutive frames with the samples for each
    // channel interleaved.
    //
    // Returns the actual number of frames that have been read which
    // may be lower than the requested number of frames.
    virtual size_type readFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer) = 0;

    // Utility function for explicitly reading stereo (= 2 channels)
    // frames from an AudioSource.
    //
    // If this source provides only a single channel (mono) the samples
    // of that channel will be expanded. If the source provides more
    // than 2 channels only the first 2 channels will be read.
    //
    // The minimum required capacity of the sampleBuffer is frameCount * 2.
    //
    // Derived classes may provide an optimized version that doesn't require
    // any post-processing as done by this rather inefficient default
    // implementation.
    virtual size_type readStereoFrameSamplesInterleaved(size_type frameCount,
            sample_type* sampleBuffer);

protected:
    AudioSource();

private:
    size_type m_channelCount;
    size_type m_sampleRate;
    size_type m_frameCount;
};

typedef QSharedPointer<AudioSource> AudioSourcePointer;

} // namespace Mixxx

#endif // MIXXX_AUDIOSOURCE_H
