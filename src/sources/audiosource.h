#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "sources/urlresource.h"
#include "util/audiosignal.h"
#include "util/memory.h"
#include "util/result.h"
#include "util/samplebuffer.h"

namespace mixxx {

// forward declaration(s)
class AudioSourceConfig;

// Common interface and base class for audio sources.
//
// Both the number of channels and the sampling rate must
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
class AudioSource: public UrlResource, public AudioSignal {
  public:
    static constexpr SampleLayout kSampleLayout = SampleLayout::Interleaved;

    // Returns the total number of sample frames.
    inline SINT getFrameCount() const {
        return m_frameCount;
    }

    inline bool isEmpty() const {
        return kFrameCountZero >= getFrameCount();
    }

    // The actual duration in seconds.
    // Well defined only for valid files!
    inline bool hasDuration() const {
        return hasValidSamplingRate();
    }
    inline double getDuration() const {
        DEBUG_ASSERT(hasDuration()); // prevents division by zero
        return double(getFrameCount()) / double(getSamplingRate());
    }

    // The bitrate is optional and measured in kbit/s (kbps).
    // It depends on the metadata and decoder if a value for the
    // bitrate is available.
    inline bool hasBitrate() const {
        return kBitrateZero < m_bitrate;
    }
    inline SINT getBitrate() const {
        return m_bitrate;
    }

    // Index of the first sample frame.
    inline static SINT getMinFrameIndex() {
        return kFrameIndexMin;
    }

    // Index of the sample frame following the last
    // sample frame.
    inline SINT getMaxFrameIndex() const {
        return getMinFrameIndex() + getFrameCount();
    }

    // The sample frame index is valid in the range
    // [getMinFrameIndex(), getMaxFrameIndex()].
    inline bool isValidFrameIndex(SINT frameIndex) const {
        return (getMinFrameIndex() <= frameIndex) &&
                (getMaxFrameIndex() >= frameIndex);
    }

    // Adjusts the current frame seek index:
    // - Precondition: isValidFrameIndex(frameIndex) == true
    //   - Index of first frame: frameIndex = 0
    //   - Index of last frame: frameIndex = getFrameCount() - 1
    // - The seek position in seconds is frameIndex / getSamplingRate()
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
        return readSampleFrames(numberOfFrames, static_cast<CSAMPLE*>(nullptr));
    }

    inline SINT readSampleFrames(
            SINT numberOfFrames,
            SampleBuffer* pSampleBuffer) {
        if (pSampleBuffer) {
            DEBUG_ASSERT(frames2samples(numberOfFrames) <= pSampleBuffer->size());
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

    // Utility function to clamp the frame index interval
    // [*pMinFrameIndexOfInterval, *pMaxFrameIndexOfInterval)
    // to valid frame indexes. The lower bound is inclusive and
    // the upper bound is exclusive!
    static void clampFrameInterval(
            SINT* pMinFrameIndexOfInterval,
            SINT* pMaxFrameIndexOfInterval,
            SINT maxFrameIndexOfAudioSource);

    bool verifyReadable() const override;

  protected:
    explicit AudioSource(const QUrl& url);
    explicit AudioSource(const AudioSource& other) = default;

    inline static bool isValidFrameCount(SINT frameCount) {
        return kFrameCountZero <= frameCount;
    }
    void setFrameCount(SINT frameCount);

    inline static bool isValidBitrate(SINT bitrate) {
        return kBitrateZero <= bitrate;
    }
    void setBitrate(SINT bitrate);

    SINT getSampleBufferSize(
            SINT numberOfFrames,
            bool readStereoSamples = false) const;

  private:
    friend class AudioSourceConfig;

    static constexpr SINT kFrameCountZero = 0;
    static constexpr SINT kFrameCountDefault = kFrameCountZero;

    // 0-based indexing of sample frames
    static constexpr SINT kFrameIndexMin = 0;

    static constexpr SINT kBitrateZero = 0;
    static constexpr SINT kBitrateDefault = kBitrateZero;

    SINT m_frameCount;

    SINT m_bitrate;
};

// Parameters for configuring audio sources
class AudioSourceConfig : public AudioSignal {
  public:
    AudioSourceConfig()
        : AudioSignal(AudioSource::kSampleLayout) {
    }
    AudioSourceConfig(SINT channelCount, SINT samplingRate)
        : AudioSignal(AudioSource::kSampleLayout, channelCount, samplingRate) {
    }

    using AudioSignal::setChannelCount;
    using AudioSignal::resetChannelCount;

    using AudioSignal::setSamplingRate;
    using AudioSignal::resetSamplingRate;
};

typedef std::shared_ptr<AudioSource> AudioSourcePointer;

} // namespace mixxx

#endif // MIXXX_AUDIOSOURCE_H
