#ifndef MIXXX_SAMPLEFRAMESOURCE_H
#define MIXXX_SAMPLEFRAMESOURCE_H

#include "util/audiosignal.h"
#include "util/indexrange.h"
#include "util/samplebuffer.h"


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


class ISampleFrameSource {
  public:
    virtual ~ISampleFrameSource() = default;

    enum class ReadMode {
        Store, // write/copy decoded sample data into buffer
        Skip,  // discard decoded sample data
    };

    virtual ReadableSampleFrames readSampleFramesClamped(
            ReadMode readMode,
            WritableSampleFrames sampleFrames) = 0;

    virtual IndexRange skipSampleFramesClamped(
            IndexRange frameIndexRange) {
        return readSampleFramesClamped(
                ReadMode::Skip,
                WritableSampleFrames(frameIndexRange)).frameIndexRange();
    }

};



class SampleFrameSource: public AudioSignal, public virtual ISampleFrameSource {
  public:
    virtual ~SampleFrameSource() = default;

    SampleFrameSource& operator=(const SampleFrameSource&) = delete;
    SampleFrameSource& operator=(SampleFrameSource&&) = delete;


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


    bool verifyReadable() const override;


    ReadableSampleFrames readSampleFrames(
            ReadMode readMode,
            WritableSampleFrames sampleFrames) {
        const auto sampleFramesClamped =
                clampWritableSampleFrames(readMode, sampleFrames);
        if (sampleFramesClamped.frameIndexRange().empty()) {
            return ReadableSampleFrames(
                    sampleFramesClamped.frameIndexRange());
        } else {
            return readSampleFramesClamped(
                    readMode,
                    sampleFramesClamped);
        }
    }

    IndexRange skipSampleFrames(
            IndexRange frameIndexRange) {
        const auto frameIndexRangeClamped =
                clampFrameIndexRange(frameIndexRange);
        if (frameIndexRangeClamped.empty()) {
            return frameIndexRangeClamped;
        } else {
            return skipSampleFramesClamped(
                    frameIndexRangeClamped);
        }
    }

  protected:
    SampleFrameSource()
          : AudioSignal(kSampleLayout) {
    }
    SampleFrameSource(const SampleFrameSource& other) = default;

    bool initFrameIndexRangeOnce(
            IndexRange frameIndexRange);

  private:
    friend class LegacyAudioSourceAdapter;
    WritableSampleFrames clampWritableSampleFrames(
            ReadMode readMode,
            WritableSampleFrames sampleFrames) const;
    IndexRange clampFrameIndexRange(
            IndexRange frameIndexRange) const {
        return intersect(frameIndexRange, this->frameIndexRange());
    }

    IndexRange m_frameIndexRange;

};

} // namespace mixxx


#endif // MIXXX_SAMPLEFRAMESOURCE_H
