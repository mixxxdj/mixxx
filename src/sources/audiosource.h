#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "urlresource.h"

#include "samplebuffer.h"

#include "util/assert.h"
#include "util/defs.h" // Result

#include <QSharedPointer>

#include <cstddef> // size_t / diff_t

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
    typedef std::size_t size_type;
    typedef std::ptrdiff_t diff_type;

    static const size_type kChannelCountZero = 0;
    static const size_type kChannelCountMono = 1;
    static const size_type kChannelCountStereo = 2;
    static const size_type kChannelCountDefault = kChannelCountZero;

    static const size_type kFrameRateZero = 0;
    static const size_type kFrameRateDefault = kFrameRateZero;

    static const size_type kFrameCountZero = 0;
    static const size_type kFrameCountDefault = kFrameCountZero;

    // 0-based indexing of sample frames
    static const diff_type kFrameIndexMin = 0;

    static const CSAMPLE kSampleValueZero;
    static const CSAMPLE kSampleValuePeak;

    static const size_type kBitrateZero = 0;
    static const size_type kBitrateDefault = kBitrateZero;

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

    // The optional bitrate in kbit/s (kbps).
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
        DEBUG_ASSERT(isChannelCountValid()); DEBUG_ASSERT(0 == (sampleCount % getChannelCount()));
        return sampleCount / getChannelCount();
    }

    // Index of the first sample frame.
    diff_type getFrameIndexMin() const {
        return kFrameIndexMin;
    }

    // Index of the sample frame following the last
    // sample frame.
    diff_type getFrameIndexMax() const {
        return kFrameIndexMin + getFrameCount();
    }

    // The sample frame index is valid in the range
    // [getFrameIndexMin(), getFrameIndexMax()].
    inline bool isValidFrameIndex(diff_type frameIndex) const {
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
    virtual diff_type seekSampleFrame(diff_type frameIndex) = 0;

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
    virtual size_type readSampleFrames(
            size_type numberOfFrames,
            CSAMPLE* sampleBuffer) = 0;

    inline size_type skipSampleFrames(
            size_type numberOfFrames) {
        return readSampleFrames(numberOfFrames, static_cast<CSAMPLE*>(NULL));
    }

    inline size_type readSampleFrames(
            size_type numberOfFrames,
            SampleBuffer* pSampleBuffer) {
        if (pSampleBuffer) {
            DEBUG_ASSERT(frames2samples(numberOfFrames) <= size_type(pSampleBuffer->size()));
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
    virtual size_type readSampleFramesStereo(
            size_type numberOfFrames,
            CSAMPLE* sampleBuffer,
            size_type sampleBufferSize);

    inline size_type readSampleFramesStereo(
            size_type numberOfFrames,
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

    static AudioSourcePointer onCreate(AudioSource* pNewAudioSource);

    virtual Result postConstruct() /*override*/ = 0;

    void setChannelCount(size_type channelCount);
    void setFrameRate(size_type frameRate);
    void setFrameCount(size_type frameCount);

    inline void setBitrate(size_type bitrate) {
        m_bitrate = bitrate;
    }

    size_type getSampleBufferSize(
            size_type numberOfFrames,
            bool readStereoSamples = false) const;

private:
    size_type m_channelCount;
    size_type m_frameRate;
    size_type m_frameCount;

    size_type m_bitrate;
};

} // namespace Mixxx

#endif // MIXXX_AUDIOSOURCE_H
