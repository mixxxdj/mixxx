#pragma once

#include "util/samplebuffer.h"
#include "util/types.h"

// The RingDelayBuffer is designed to be single-threaded
// and based on that IS NOT THREAD-SAFE.

/// The class implements the ring buffer with specific constraints,
/// features and optimizations for the "delay handling" use case.
/// The implementation is preferably designed for delay handling
/// of the effect chain which is implemented
/// in "engine/effects/engineeffectsdelay.cpp(.h)".

/// The first main difference between the "classic" ring buffer
/// as widely known and this one is, that the first input items chunk,
/// that is written into the empty ring buffer is ramped (faded-in).
/// This feature is implemented into the ring buffer due to avoid "crackling"
/// when the reader crossed the border between zero silence samples
/// and the real data that was written into the ring buffer. This situation
/// is possible in the mentioned usage of the effect chain delay handling.

/// Based on all of that, the second difference is, that the ring buffer
/// is prefilled with zero "silence" samples in the default state or after
/// clean-up to avoid the reading of uninitialized data.

/// The last main difference is, that the ring buffer stores
/// only the information about the write position, not about the read position.
/// The items are read using the write position, the number of items
/// that should be read and the delay of the reading position
/// against the write position. The reading is optimized this way
/// for the use case of the effect chain delay handling.
class RingDelayBuffer final {
  public:
    RingDelayBuffer(SINT bufferSize);

    /// The method clears the ring delay buffer. That means,
    /// that the ring buffer is filled with zero "silence" samples,
    /// the write position is at the start of the ring buffer (index 0)
    /// and as last, the flag for the first input buffer is set in the state,
    /// that the next chunk of data that will be written, will be faded-in.
    void clear() {
        m_firstInputChunk = true;
        m_writePos = 0;

        m_buffer.fill(0);
    }

    /// The method returns the size of the ring buffer.
    constexpr SINT size() const {
        return m_buffer.size();
    }

    /// The method reads items from the ring buffer and stores them
    /// in the pBuffer. The number of items that will be read
    /// is passed through the itemsToRead parameter. To allow delay handling
    /// when reading ring buffer data, the delayItems parameter contains
    /// the delay items that determine the reading position
    /// against the write position. The sum of items to read and the delay items
    /// has to be smaller or equal to the ring buffer size. Otherwise, the items
    /// are not read and the method returns the zero value.
    SINT read(std::span<CSAMPLE> destinationBuffer, const SINT delayItems);

    /// The method writes items from the pBuffer into the ring buffer.
    /// The number of items that will be written is passed through
    /// the itemsToWrite parameter. This value has to be smaller or equal
    /// to the ring buffer size. Otherwise, the items are not written
    /// and the method returns the zero value. The first chunk of items
    /// after creating of ring delay buffer or after clean-up is ramped
    /// by using fading-in.
    SINT write(std::span<const CSAMPLE> sourceBuffer);

  private:
    // This flag ensures the "fading in" for the first input chunk
    // into the clear ring delay buffer. It is done to avoid
    // a "crackling" sound when the input samples are read after reading
    // the previous zero samples.
    bool m_firstInputChunk;
    // Position of next writable element.
    SINT m_writePos;
    // Ring delay buffer.
    mixxx::SampleBuffer m_buffer;
};
