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

    void ensureMinCapacity(
            FrameCount minCapacity);

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

    /// Try to reposition the buffer to a new read position
    /// within the buffered range, keeping all remaining data
    /// ahead of the read position.
    bool tryContinueReadingFrom(
            FrameIndex readIndex);

    void reset(
            FrameIndex currentIndex = kUnknownFrameIndex);

    /// Advance the read position thereby discarding samples
    /// from the front of the FIFO buffer.
    FrameCount discardFirstBufferedFrames(
            FrameCount frameCount);

    /// Rewind the write position thereby discarding samples
    /// from the back of the FIFO buffer.
    FrameCount discardLastBufferedFrames(
            FrameCount frameCount);

    enum class BufferingMode {
        SkipGap,
        FillGapWithSilence,
    };

    /// Buffer the given readable samples frames
    ///
    /// The given sample data must start after the last buffered
    /// frame.
    ///
    /// Returns the unread portion of the readable sample frames,
    /// which should typically be empty.
    ReadableSampleFrames bufferSampleData(
            BufferingMode bufferingMode,
            ReadableSampleFrames inputSampleFrames);

    /// Consume as many buffered sample frames as possible
    ///
    /// Returns the remaining portion that could not be filled from
    /// the buffer.
    WritableSampleFrames consumeBufferedSampleData(
            WritableSampleFrames outputSampleFrames);

    void discardAllBufferedSampleData() {
        discardFirstBufferedFrames(bufferedLength());
    }

  private:
    audio::SignalInfo m_signalInfo;
    ReadAheadSampleBuffer m_sampleBuffer;
    FrameIndex m_readIndex;
};

} // namespace mixxx
