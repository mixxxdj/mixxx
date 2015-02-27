#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "urlresource.h"

#include "samplebuffer.h"

#include "util/assert.h"
#include "util/defs.h" // Result

#include <QSharedPointer>

namespace Mixxx {

class AudioSource;
typedef QSharedPointer<AudioSource> AudioSourcePointer;

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
class AudioSource: public UrlResource {
public:
    static const SINT kChannelCountZero = 0;
    static const SINT kChannelCountMono = 1;
    static const SINT kChannelCountStereo = 2;
    static const SINT kChannelCountDefault = kChannelCountZero;

    static const SINT kFrameRateZero = 0;
    static const SINT kFrameRateDefault = kFrameRateZero;

    static const SINT kFrameCountZero = 0;
    static const SINT kFrameCountDefault = kFrameCountZero;

    // 0-based indexing of sample frames
    static const SINT kFrameIndexMin = 0;

    static const CSAMPLE kSampleValueZero;
    static const CSAMPLE kSampleValuePeak;

    static const SINT kBitrateZero = 0;
    static const SINT kBitrateDefault = kBitrateZero;

    // Returns the number of channels. The number of channels
    // must be constant over time.
    inline SINT getChannelCount() const {
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
    inline SINT getFrameRate() const {
        return m_frameRate;
    }
    inline bool isFrameRateValid() const {
        return kFrameRateZero < getFrameRate();
    }

    // Returns the total number of frames.
    inline SINT getFrameCount() const {
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

    // The optional bitrate in kbit/s (kbps).
    // Derived classes may set the actual (average) bitrate when
    // opening the file. The bitrate is not needed for decoding,
    // it is only used for informational purposes.
    inline bool hasBitrate() const {
        return kBitrateZero < m_bitrate;
    }
    inline SINT getBitrate() const {
        return m_bitrate;
    }

    // The actual duration in seconds.
    // Only available for valid files!
    inline bool hasDuration() const {
        return isValid();
    }
    inline SINT getDuration() const {
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
        DEBUG_ASSERT(isChannelCountValid()); DEBUG_ASSERT(0 == (sampleCount % getChannelCount()));
        return sampleCount / getChannelCount();
    }

    // Index of the first sample frame.
    SINT getFrameIndexMin() const {
        return kFrameIndexMin;
    }

    // Index of the sample frame following the last
    // sample frame.
    SINT getFrameIndexMax() const {
        return kFrameIndexMin + getFrameCount();
    }

    // The sample frame index is valid in the range
    // [getFrameIndexMin(), getFrameIndexMax()].
    inline bool isValidFrameIndex(SINT frameIndex) const {
        return (getFrameIndexMin() <= frameIndex) &&
                (getFrameIndexMax() >= frameIndex);
    }

    // Adjusts the current frame seek index:
    // - Precondition: isValidFrameIndex(frameIndex) == true
    //   - Index of first frame: frameIndex = 0
    //   - Index of last frame: frameIndex = getFrameCount() - 1
    // - The seek position in seconds is frameIndex / frameRate()
    // Returns the actual current frame index which may differ from the
    // requested index if the source does not support accurate seeking.
    virtual SINT seekSampleFrame(SINT frameIndex) = 0;

    // Fills the buffer with samples from each channel starting
    // at the current frame seek position.
    //
    // The implicit  minimum required capacity of the sampleBuffer is
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

    inline SINT skipSampleFrames(
            SINT numberOfFrames) {
        return readSampleFrames(numberOfFrames, static_cast<CSAMPLE*>(NULL));
    }

    inline SINT readSampleFrames(
            SINT numberOfFrames,
            SampleBuffer* pSampleBuffer) {
        if (pSampleBuffer) {
            DEBUG_ASSERT(frames2samples(numberOfFrames) <= SINT(pSampleBuffer->size()));
            return readSampleFrames(numberOfFrames, pSampleBuffer->data());
        } else {
            return skipSampleFrames(numberOfFrames);
        }
    }

    // Specialized function for explicitly reading stereo (= 2 channels)
    // frames from an AudioSource. This is the preferred method in Mixxx
    // to read a stereo signal.
    //
    // If the source provides only a single channel (mono) the samples
    // of that channel will be doubled. If the source provides more
    // than 2 channels only the first 2 channels will be read.
    //
    // Most audio sources in Mixxx implicitly reduce multi-channel output
    // to stereo during decoding. Other audio sources override this method
    // with an optimized version that does not require a second pass through
    // the sample data or that avoids the allocation of a temporary buffer
    // when reducing multi-channel data to stereo.
    // 
    // The minimum required capacity of the sampleBuffer is
    //     sampleBufferSize = numberOfFrames * 2
    // In order to avoid the implicit allocation of a temporary buffer
    // when reducing multi-channel to stereo the caller must provide
    // a sample buffer of size
    //     sampleBufferSize = frames2samples(numberOfFrames)
    //
    // Returns the actual number of frames that have been read which
    // might be lower than the requested number of frames when the end
    // of the audio stream has been reached. The current frame seek
    // position is moved forward towards the next unread frame.
    //
    // Derived classes may provide an optimized version that doesn't
    // require any post-processing as done by this default implementation.
    // They may also have reduced space requirements on sampleBuffer,
    // i.e. only the minimum size is required for an in-place
    // transformation without temporary allocations.
    virtual SINT readSampleFramesStereo(
            SINT numberOfFrames,
            CSAMPLE* sampleBuffer,
            SINT sampleBufferSize);

    inline SINT readSampleFramesStereo(
            SINT numberOfFrames,
            SampleBuffer* pSampleBuffer) {
        if (pSampleBuffer) {
            return readSampleFramesStereo(numberOfFrames,
                    pSampleBuffer->data(), pSampleBuffer->size());
        } else {
            return skipSampleFrames(numberOfFrames);
        }
    }

protected:
    explicit AudioSource(QUrl url);

    void setChannelCount(SINT channelCount);
    void setFrameRate(SINT frameRate);
    void setFrameCount(SINT frameCount);

    inline void setBitrate(SINT bitrate) {
        m_bitrate = bitrate;
    }

    SINT getSampleBufferSize(
            SINT numberOfFrames,
            bool readStereoSamples = false) const;

private:
    SINT m_channelCount;
    SINT m_frameRate;
    SINT m_frameCount;

    SINT m_bitrate;
};

} // namespace Mixxx

#endif // MIXXX_AUDIOSOURCE_H
