#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "util/assert.h"
#include "util/types.h" // CSAMPLE

#include <QSharedPointer>

#include <cstddef> // size_t / diff_t

namespace Mixxx {

// Common interface and base class for audio sources.
//
// Both the number of channels and the frame rate must
// be constant and are not allowed to change over time.
//
// The length of audio data is measured in frames. A frame
// is a tuple containing samples from each channel that are
// coincident in time. A frame for a mono signal contains a
// single sample. A frame for a stereo signal contains a pair
// of samples, one for the left and right channel respectively.
//
// Samples in a sample buffer are stored as consecutive frames,
// i.e. the samples of the channels are interleaved.
//
// Audio sources are implicitly opened upon creation and
// closed upon destruction.
class AudioSource {
public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t diff_type;
    typedef CSAMPLE sample_type;

    static const size_type kChannelCountZero = 0;
    static const size_type kChannelCountMono = 1;
    static const size_type kChannelCountStereo = 2;
    static const size_type kChannelCountDefault = kChannelCountZero;

    static const size_type kFrameRateZero = 0;
    static const size_type kFrameRateDefault = kFrameRateZero;

    static const size_type kFrameCountZero = 0;
    static const size_type kFrameCountDefault = kFrameCountZero;

    static const sample_type kSampleValueZero;
    static const sample_type kSampleValuePeak;

    static const size_type kBitrateZero = 0;
    static const size_type kBitrateDefault = kBitrateZero;

    virtual ~AudioSource();

    // Returns the number of channels. The number of channels
    // must be constant over time.
    inline size_type getChannelCount() const {
        return m_channelCount;
    }
    inline bool isChannelCountValid() const {
        return kChannelCountZero < getChannelCount();
    }
    inline bool isChannelCountMono() const {
        return kChannelCountMono == getChannelCount();
    }
    inline bool isChannelCountStereo() const {
        return kChannelCountStereo == getChannelCount();
    }

    // Returns the number of frames per second. This equals
    // the number samples for each channel per second, which
    // must be uniform among all channels. The frame rate
    // must be constant over time.
    inline size_type getFrameRate() const {
        return m_frameRate;
    }
    inline bool isFrameRateValid() const {
        return kFrameRateZero < getFrameRate();
    }

    // Returns the total number of frames.
    inline size_type getFrameCount() const {
        return m_frameCount;
    }
    inline bool isFrameCountEmpty() const {
        return kFrameCountZero >= getFrameCount();
    }

    inline bool isValid() const {
        return isChannelCountValid() && isFrameRateValid();
    }

    inline bool isEmpty() const {
        return isFrameCountEmpty();
    }

    // The bitrate in kbit/s (optional).
    // Derived classes may set the actual (average) bitrate when
    // opening the file. The bitrate is not needed for decoding,
    // it is only used for informational purposes.
    inline bool hasBitrate() const {
        return kBitrateZero < m_bitrate;
    }
    inline size_type getBitrate() const {
        return m_bitrate;
    }

    // The actual duration in seconds.
    // Only available for valid files!
    inline bool hasDuration() const {
        return isValid();
    }
    inline size_type getDuration() const {
        DEBUG_ASSERT(hasDuration()); // prevents division by zero
        return getFrameCount() / getFrameRate();
    }

    // Conversion: #frames -> #samples
    template<typename T>
    inline T frames2samples(T frameCount) const {
        DEBUG_ASSERT(isChannelCountValid());
        return frameCount * getChannelCount();
    }

    // Conversion: #samples -> #frames
    template<typename T>
    inline T samples2frames(T sampleCount) const {
        DEBUG_ASSERT(isChannelCountValid());
        DEBUG_ASSERT(0 == (sampleCount % getChannelCount()));
        return sampleCount / getChannelCount();
    }

    // Adjusts the current frame seek index:
    // - Index of first frame: frameIndex = 0
    // - Index of last frame: frameIndex = totalFrames() - 1
    // - The seek position in seconds is frameIndex / frameRate()
    // Returns the actual current frame index which may differ from the
    // requested index if the source does not support accurate seeking.
    virtual diff_type seekSampleFrame(diff_type frameIndex) = 0;

    // Fills the buffer with samples from each channel starting
    // at the current frame seek position.
    //
    // The required size of the sampleBuffer is numberOfSamples =
    // frames2samples(numberOfFrames). The samples in the sampleBuffer
    // are stored as consecutive sample frames with samples from
    // each channel interleaved.
    //
    // Returns the actual number of frames that have been read which
    // may be lower than the requested number of frames. The current
    // frame seek position is moved forward to the next unread frame.
    virtual size_type readSampleFrames(size_type numberOfFrames,
            sample_type* sampleBuffer) = 0;

    // Specialized function for explicitly reading stereo (= 2 channels)
    // frames from an AudioSource. This is commonly used in Mixxx!
    //
    // If the source provides only a single channel (mono) the samples
    // of that channel will be doubled. If the source provides more
    // than 2 channels only the first 2 channels will be read. The
    // minimum required capacity of the sampleBuffer is numberOfFrames * 2.
    //
    // Returns the actual number of frames that have been read which
    // may be lower than the requested number of frames. The current
    // frame seek position is moved forward towards the next unread
    // frame.
    //
    // Derived classes may provide an optimized version that doesn't
    // require any post-processing as done by this default implementation.
    // Please note that the default implementation performs poorly when
    // reducing more than 2 channels to stereo!
    virtual size_type readSampleFramesStereo(size_type numberOfFrames,
            sample_type* sampleBuffer);

protected:
    AudioSource();

    void setChannelCount(size_type channelCount);
    void setFrameRate(size_type frameRate);
    void setFrameCount(size_type frameCount);

    inline void setBitrate(size_type bitrate) {
        m_bitrate = bitrate;
    }

    void reset();

private:
    size_type m_channelCount;
    size_type m_frameRate;
    size_type m_frameCount;

    size_type m_bitrate;
};

typedef QSharedPointer<AudioSource> AudioSourcePointer;

} // namespace Mixxx

#endif // MIXXX_AUDIOSOURCE_H
