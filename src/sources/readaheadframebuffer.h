#pragma once

#include <limits>

#include "sources/audiosource.h"
#include "util/readaheadsamplebuffer.h"

namespace mixxx {

typedef SINT FrameIndex;
typedef SINT FrameCount;

/// Keeps track of the current frame position (index) while decoding
/// an audio stream. Buffers samples (FIFO) ahead of this position
/// that have been decoded but not yet consumed.
class ReadAheadFrameBuffer final {
  public:
    static constexpr FrameIndex kInvalidFrameIndex = std::numeric_limits<FrameIndex>::min();
    static constexpr FrameIndex kUnknownFrameIndex = std::numeric_limits<FrameIndex>::max();

    static constexpr FrameCount kEmptyCapacity = 0;

    ReadAheadFrameBuffer();
    explicit ReadAheadFrameBuffer(
            const audio::SignalInfo& signalInfo,
            FrameCount initialCapacity = kEmptyCapacity);
    ReadAheadFrameBuffer(ReadAheadFrameBuffer&&) = default;
    ReadAheadFrameBuffer(const ReadAheadFrameBuffer&) = delete;
    ReadAheadFrameBuffer& operator=(ReadAheadFrameBuffer&&) = default;
    ReadAheadFrameBuffer& operator=(const ReadAheadFrameBuffer&) = delete;

    const audio::SignalInfo& signalInfo() const {
        return m_signalInfo;
    }

    FrameCount capacity() const {
        return m_signalInfo.samples2frames(
                m_sampleBuffer.capacity());
    }

    /// Invalidate the whole buffer and its position, e.g. to
    /// indicate decoding errors.
    void invalidate() {
        m_readIndex = kInvalidFrameIndex;
    }

    bool isValid() const {
        return m_readIndex != kInvalidFrameIndex;
    }

    /// Check that the first readable frame is at a valid position.
    bool isReady() const {
        return isValid() &&
                m_readIndex != kUnknownFrameIndex;
    }

    bool isEmpty() const {
        return m_sampleBuffer.empty();
    }

    FrameCount bufferedLength() const {
        DEBUG_ASSERT(isReady() || isEmpty());
        return m_signalInfo.samples2frames(
                m_sampleBuffer.readableLength());
    }

    IndexRange bufferedRange() const {
        return IndexRange::forward(
                readIndex(),
                bufferedLength());
    }

    /// Return the current position from where data
    /// could be read.
    FrameIndex readIndex() const {
        DEBUG_ASSERT(isValid());
        return m_readIndex;
    }

    /// Return the current position from where data
    /// will be written.
    FrameIndex writeIndex() const {
        return readIndex() + bufferedLength();
    }

    void reset(
            FrameIndex currentIndex = kUnknownFrameIndex);

    /// Try to reposition the buffer to a new read position
    /// within the buffered range, keeping all remaining data
    /// ahead of the read position.
    bool tryContinueReadingFrom(
            FrameIndex readIndex);

    enum class DiscontinuityOverlapMode {
        Ignore,
        Rewind,
        Default = Rewind, // recommended default
    };
    enum class DiscontinuityGapMode {
        Skip,
        FillWithSilence,
        Default = FillWithSilence, // recommended default
    };

    /// Drain as many buffered sample frames as possible and copy them
    /// into the output buffer.
    ///
    /// Sample data can only be consumed from the current read index
    /// or beyond.
    ///
    /// The output sample buffer may be null. In this case the consumed
    /// samples are dropped instead of copied.
    ///
    /// Returns the remaining portion that could not be filled from
    /// the buffer.
    WritableSampleFrames drainBuffer(
            WritableSampleFrames outputBuffer);

    /// Fills first the output buffer and then the internal buffer with
    /// the data from the input buffer. The whole input buffer is consumed.
    ///
    /// If the inputBuffer starts before the current position write
    /// position then the position is rewind back as far as possible.
    /// The parameter minOutputIndex indicates how far back the
    /// output buffer could be rewind, i.e. indicates the very first
    /// position before the current start position that would still
    /// be valid.
    ///
    /// All discontinuities in the output stream, i.e. both gaps and overlapping
    /// regions are unexpected and will trigger a debug assertion.
    ///
    /// The output sample buffer may be null. In this case the consumed
    /// samples are dropped instead of copied.
    ///
    /// Returns the remaining portion that could not be filled from
    /// the buffer.
    WritableSampleFrames consumeAndFillBuffer(ReadableSampleFrames inputBuffer,
            WritableSampleFrames outputBuffer,
            FrameIndex minOutputIndex,
            std::pair<DiscontinuityOverlapMode, DiscontinuityGapMode>
                    discontinuityModes = std::make_pair(
                            DiscontinuityOverlapMode::Default,
                            DiscontinuityGapMode::Default));

  private:
    void adjustCapacityBeforeBuffering(
            FrameCount frameCount);

    /// Buffer the given readable samples frames.
    ///
    /// The given sample data must start at after the last buffered
    /// frame. The buffering mode controls how a gap between the
    /// last buffered frame and the next input frame is handled.
    ///
    /// Returns the unread portion of the readable sample frames,
    /// which should typically be empty.
    ReadableSampleFrames fillBuffer(
            ReadableSampleFrames inputBuffer,
            DiscontinuityGapMode discontinuityGapMode);

    /// Advance the read position thereby discarding samples
    /// from the front of the FIFO buffer.
    FrameCount discardFirstBufferedFrames(
            FrameCount frameCount);

    /// Rewind the write position thereby discarding samples
    /// from the back of the FIFO buffer.
    FrameCount discardLastBufferedFrames(
            FrameCount frameCount);

    audio::SignalInfo m_signalInfo;
    ReadAheadSampleBuffer m_sampleBuffer;
    FrameIndex m_readIndex;
};

} // namespace mixxx
