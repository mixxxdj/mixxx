#pragma once

#include <limits>

#include "sources/audiosource.h"
#include "util/readaheadsamplebuffer.h"

namespace mixxx {

typedef SINT FrameIndex;
typedef SINT FrameCount;

/// Keeps track of the current frame position (index) while decoding
/// an audio stream. Buffers samples ahead of this position that
/// have been decoded but not yet consumed.
class ReadAheadFrameBuffer final {
    // 1 sec @ 96 kHz / 2 sec @ 48 kHz / 2.18 sec @ 44.1 kHz
    static constexpr FrameCount kMinFrameCapacity = 96000;

  public:
    static constexpr FrameIndex kInvalidFrameIndex = std::numeric_limits<FrameIndex>::min();
    static constexpr FrameIndex kUnknownFrameIndex = std::numeric_limits<FrameIndex>::max();

    ReadAheadFrameBuffer();
    explicit ReadAheadFrameBuffer(
            const audio::SignalInfo& signalInfo,
            FrameCount initialFrameCapacity = kMinFrameCapacity);
    ReadAheadFrameBuffer(ReadAheadFrameBuffer&&) = delete;
    ReadAheadFrameBuffer(const ReadAheadFrameBuffer&) = delete;

    void reinit(
            const audio::SignalInfo& signalInfo,
            FrameCount minFrameCapacity = kMinFrameCapacity);

    const audio::SignalInfo& signalInfo() const {
        return m_signalInfo;
    }

    bool isValid() const {
        return m_firstFrameIndex != kInvalidFrameIndex;
    }

    /// Check that the first frame is a valid position
    bool isReady() const {
        return isValid() &&
                m_firstFrameIndex != kUnknownFrameIndex;
    }

    /// Return the current position
    FrameIndex firstFrame() const {
        DEBUG_ASSERT(isValid());
        return m_firstFrameIndex;
    }

    bool isEmpty() const {
        return m_sampleBuffer.empty();
    }

    FrameCount frameCapacity() const {
        return m_signalInfo.samples2frames(
                m_sampleBuffer.capacity());
    }

    void ensureMinFrameCapacity(FrameCount minFrameCapacity);

    FrameCount bufferedFrameLength() const {
        DEBUG_ASSERT(isReady() || isEmpty());
        return m_signalInfo.samples2frames(
                m_sampleBuffer.readableLength());
    }

    IndexRange bufferedFrameRange() const {
        return IndexRange::forward(
                firstFrame(),
                bufferedFrameLength());
    }

    void invalidate() {
        m_firstFrameIndex = kInvalidFrameIndex;
    }

    void reset(
            FrameIndex firstFrameIndex = kUnknownFrameIndex) {
        m_firstFrameIndex = firstFrameIndex;
        m_sampleBuffer.clear();
    }

    /// Try to reposition the buffer to a new start position
    /// within the buffered frame range.
    bool trySeekToFirstFrame(FrameIndex firstFrameIndex);

    void discardAllBufferedFrames() {
        DEBUG_ASSERT(isReady());
        m_firstFrameIndex +=
                m_signalInfo.samples2frames(m_sampleBuffer.readableLength());
        m_sampleBuffer.clear();
    }

    /// Fast forward
    FrameCount discardFirstBufferedFrames(FrameCount frameCount);

    /// Rewind
    FrameCount discardLastBufferedFrames(FrameCount frameCount);

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
    ReadableSampleFrames bufferFrames(
            BufferingMode bufferingMode,
            ReadableSampleFrames readableSampleFrames);

    /// Consume as many buffered sample frames as possible
    ///
    /// Returns the remaining portion that could not be filled from
    /// the buffer.
    WritableSampleFrames consumeBufferedFrames(
            WritableSampleFrames writableSampleFrames);

  private:
    audio::SignalInfo m_signalInfo;
    ReadAheadSampleBuffer m_sampleBuffer;
    FrameIndex m_firstFrameIndex;
};

} // namespace mixxx
