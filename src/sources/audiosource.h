#ifndef MIXXX_AUDIOSOURCE_H
#define MIXXX_AUDIOSOURCE_H

#include "sources/urlresource.h"

#include "util/audiosignal.h"
#include "util/indexrange.h"
#include "util/samplebuffer.h"
#include "util/memory.h"


namespace mixxx {

class SampleFrames {
  public:
    SampleFrames() = default;
    explicit SampleFrames(
            IndexRange frameIndexRange)
        : m_frameIndexRange(frameIndexRange) {
    }
    /*non-virtual*/ ~SampleFrames() = default;

    IndexRange frameIndexRange() const {
        return m_frameIndexRange;
    }

  private:
    IndexRange m_frameIndexRange;
};


class ReadableSampleFrames: public SampleFrames {
  public:
    ReadableSampleFrames() = default;
    explicit ReadableSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::ReadableSlice sampleBuffer = SampleBuffer::ReadableSlice())
          : SampleFrames(frameIndexRange),
            m_sampleBuffer(sampleBuffer) {
    }
    /*non-virtual*/ ~ReadableSampleFrames() = default;

    // The readable slice should cover the whole range of
    // frame indices and starts with the first frame. An
    // empty slice indicates that no sample data is available
    // for reading.
    SampleBuffer::ReadableSlice sampleBuffer() const {
        return m_sampleBuffer;
    }

  private:
    SampleBuffer::ReadableSlice m_sampleBuffer;
};


class WritableSampleFrames: public SampleFrames {
  public:
    WritableSampleFrames() = default;
    explicit WritableSampleFrames(
            IndexRange frameIndexRange,
            SampleBuffer::WritableSlice sampleBuffer = SampleBuffer::WritableSlice())
          : SampleFrames(frameIndexRange),
            m_sampleBuffer(sampleBuffer) {
    }
    /*non-virtual*/ ~WritableSampleFrames() = default;

    // The writable slice should cover the whole range of
    // frame indices and starts with the first frame. An
    // empty slice indicates that no sample data must
    // be written.
    SampleBuffer::WritableSlice sampleBuffer() const {
        return m_sampleBuffer;
    }

  private:
    SampleBuffer::WritableSlice m_sampleBuffer;
};


// Interface for reading audio data in sample frames.
//
// Each new type of source must implement at least readSampleFramesClamped().
class IAudioSource {
  public:
    virtual ~IAudioSource() = default;

  protected:
    // Reads as much of the the requested sample frames and writes
    // them into the provided buffer. The capacity of the buffer
    // and the requested range have already been checked and
    // adjusted (= clamped) before if necessary.
    //
    // Returns the number of and decoded sample frames in a readable
    // buffer. The returned buffer is just a view/slice of the provided
    // writable buffer if the result is not empty. If the result is
    // empty the internal memory pointer of the returned buffer might
    // be null.
    virtual ReadableSampleFrames readSampleFramesClamped(
            WritableSampleFrames sampleFrames) = 0;

    // The following function is required for accessing the protected
    // read function from siblings implementing this interface, e.g.
    // for proxies and adapters.
    static ReadableSampleFrames readSampleFramesClampedOn(
            IAudioSource& that,
            WritableSampleFrames sampleFrames) {
        return that.readSampleFramesClamped(sampleFrames);
    }

};


// Common base class for audio sources.
//
// Both the number of channels and the sampling rate must
// be constant and are not allowed to change over time.
//
// Samples in a sample buffer are stored as consecutive frames,
// i.e. the samples of the channels are interleaved.
//
// Audio sources are implicitly opened upon creation and
// closed upon destruction.
class AudioSource: public UrlResource, public AudioSignal, public virtual IAudioSource {
  public:
    virtual ~AudioSource() = default;


    // All sources are required to produce a signal of frames
    // where each frame contains samples from all channels that are
    // coincident in time.
    //
    // A frame for a mono signal contains a single sample. A frame
    // for a stereo signal contains a pair of samples, one for the
    // left and right channel respectively.
    static constexpr SampleLayout kSampleLayout = SampleLayout::Interleaved;


    // The total length of audio data is bounded and measured in frames.
    IndexRange frameIndexRange() const {
        return m_frameIndexRange;
    }

    // The index of the first frame.
    SINT frameIndexMin() const {
        DEBUG_ASSERT(m_frameIndexRange.start() <= m_frameIndexRange.end());
        return m_frameIndexRange.start();
    }

    // The index after the last frame.
    SINT frameIndexMax() const {
        DEBUG_ASSERT(m_frameIndexRange.start() <= m_frameIndexRange.end());
        return m_frameIndexRange.end();
    }

    // The sample frame index is valid within the range
    // [frameIndexMin(), frameIndexMax()]
    // including the upper bound of the range!
    bool isValidFrameIndex(SINT frameIndex) const {
        return m_frameIndexRange.clampIndex(frameIndex) == frameIndex;
    }


    // The actual duration in seconds.
    // Well defined only for valid files!
    inline bool hasDuration() const {
        return samplingRate().valid();
    }
    inline double getDuration() const {
        DEBUG_ASSERT(hasDuration()); // prevents division by zero
        return double(frameIndexRange().length()) / double(samplingRate());
    }


    // The bitrate is optional and measured in kbit/s (kbps).
    // It depends on the metadata and decoder if a value for the
    // bitrate is available.
    class Bitrate {
      private:
        static constexpr SINT kValueDefault = 0;

      public:
        static constexpr const char* unit() { return "kbps"; }

        explicit constexpr Bitrate(SINT value = kValueDefault)
            : m_value(value) {
        }

        bool valid() const {
            return m_value > kValueDefault;
        }

        /*implicit*/ operator SINT() const {
            DEBUG_ASSERT(m_value >= kValueDefault); // unsigned value
            return m_value;
        }

      private:
        SINT m_value;
    };

    Bitrate bitrate() const {
        return m_bitrate;
    }


    bool verifyReadable() const override;


    ReadableSampleFrames readSampleFrames(
            WritableSampleFrames sampleFrames) {
        const auto sampleFramesFramesClamped =
                clampWritableSampleFrames(sampleFrames);
        if (sampleFramesFramesClamped.frameIndexRange().empty()) {
            // result is empty
            return ReadableSampleFrames(
                    sampleFramesFramesClamped.frameIndexRange());
        } else {
            // forward clamped request
            return readSampleFramesClamped(
                    sampleFramesFramesClamped);
        }
    }

  protected:
    explicit AudioSource(const QUrl& url);
    AudioSource(const AudioSource&) = default;

    bool initFrameIndexRangeOnce(
            IndexRange frameIndexRange);

    bool initBitrateOnce(Bitrate bitrate);
    bool initBitrateOnce(SINT bitrate) {
        return initBitrateOnce(Bitrate(bitrate));
    }

  private:
    AudioSource(AudioSource&&) = delete;
    AudioSource& operator=(const AudioSource&) = delete;
    AudioSource& operator=(AudioSource&&) = delete;

    WritableSampleFrames clampWritableSampleFrames(
            WritableSampleFrames sampleFrames) const;
    IndexRange clampFrameIndexRange(
            IndexRange frameIndexRange) const {
        return intersect(frameIndexRange, this->frameIndexRange());
    }

    IndexRange m_frameIndexRange;

    Bitrate m_bitrate;
};

// Parameters for configuring audio sources
class AudioSourceConfig : public AudioSignal {
  public:
    AudioSourceConfig()
        : AudioSignal(AudioSource::kSampleLayout) {
    }
    AudioSourceConfig(ChannelCount channelCount, SamplingRate samplingRate)
        : AudioSignal(AudioSource::kSampleLayout, channelCount, samplingRate) {
    }

    using AudioSignal::setChannelCount;
    using AudioSignal::setSamplingRate;
};

typedef std::shared_ptr<AudioSource> AudioSourcePointer;

} // namespace mixxx


#endif // MIXXX_AUDIOSOURCE_H
