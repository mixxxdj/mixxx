#include "ringdelaybuffer.h"

#include "util/math.h"
#include "util/sample.h"

namespace {
// The helper function for copying data by using one ring buffer
// and one contiguous buffer or using two contiguous buffers.
// The situation working with two ring buffers is not allowed
// due to this situation is not possible with delay handling
// and for this situation, there is a need for three copies.
// If the copying will come across the upper bound of the ring buffer,
// the next items are copied from/to the start of the ring buffer.
SINT copyRing(const std::span<const CSAMPLE> sourceBuffer,
        SINT sourcePos,
        const std::span<CSAMPLE> destBuffer,
        SINT destPos,
        const SINT numItems) {
    const unsigned int newSourcePos = sourcePos + numItems;
    const unsigned int newDestPos = destPos + numItems;

    SINT sourceRemainingItems = sourceBuffer.size() - sourcePos;
    SINT destRemainingItems = destBuffer.size() - destPos;

    // Do not allow the new positions will both cross the upper bound
    // of their buffers. Based on that, the three copies will be needed,
    // but for the delay handling use case of the ring buffer,
    // this situation is not possible and is not allowed
    // in this helper function.
    VERIFY_OR_DEBUG_ASSERT(newSourcePos <= sourceBuffer.size() ||
            newDestPos <= destBuffer.size()) {
        return 0;
    }

    // Check to see if the copy is not contiguous.
    if (newSourcePos > sourceBuffer.size() ||
            newDestPos > destBuffer.size()) {
        // Copy is not contiguous.
        SINT firstDataBlockSize = math_min(sourceRemainingItems, destRemainingItems);

        // Copy the first data part until the end of the destination
        // or the source buffer.
        SampleUtil::copy(destBuffer.last(destRemainingItems).data(),
                sourceBuffer.last(sourceRemainingItems).data(),
                firstDataBlockSize);

        // Calculate new source and destination position.
        sourcePos = (sourcePos + firstDataBlockSize) % sourceBuffer.size();
        destPos = (destPos + firstDataBlockSize) % destBuffer.size();

        // Based on the new source and destination positions recalculate
        // the remaining items from the new positions.
        sourceRemainingItems = sourceBuffer.size() - sourcePos;
        destRemainingItems = destBuffer.size() - destPos;

        // The second data part is the start of the source
        // or destination buffer.
        SampleUtil::copy(destBuffer.last(destRemainingItems).data(),
                sourceBuffer.last(sourceRemainingItems).data(),
                numItems - firstDataBlockSize);
    } else {
        // Copy is contiguous.
        SampleUtil::copy(destBuffer.last(destRemainingItems).data(),
                sourceBuffer.last(sourceRemainingItems).data(),
                numItems);
    }

    return numItems;
}

} // anonymous namespace

RingDelayBuffer::RingDelayBuffer(SINT bufferSize)
        : m_firstInputChunk(true),
          m_writePos(0),
          m_buffer(bufferSize) {
    // Set the ring buffer items to 0.
    m_buffer.fill(0);
}

SINT RingDelayBuffer::read(std::span<CSAMPLE> destinationBuffer, const SINT delayItems) {
    const SINT itemsToRead = destinationBuffer.size();
    const SINT shift = itemsToRead + delayItems;

    // The reading position shift against the write position
    // has to be smaller or equal to the ring buffer size.
    VERIFY_OR_DEBUG_ASSERT(shift <= m_buffer.size()) {
        return 0;
    }

    SINT readPos = m_writePos - shift;

    // The reading position crossed the left bound of the ring buffer.
    // Add the size of the ring buffer to move the position around and keep it
    // in the valid index range.
    if (readPos < 0) {
        readPos = readPos + m_buffer.size();
    }

    return copyRing(mixxx::spanutil::spanFromPtrLen(m_buffer.data(), m_buffer.size()),
            readPos,
            destinationBuffer,
            0,
            itemsToRead);
}

SINT RingDelayBuffer::write(std::span<const CSAMPLE> sourceBuffer) {
    const SINT itemsToWrite = sourceBuffer.size();

    VERIFY_OR_DEBUG_ASSERT(itemsToWrite <= m_buffer.size()) {
        return 0;
    }

    const SINT numItems = [&]() {
        if (m_firstInputChunk) {
            // If the first input chunk is written, the first sample is on the index 0.
            // Based on the checking of an available number of samples, the situation,
            // that the writing will be non-contiguous cannot occur.
            SampleUtil::copyWithRampingGain(m_buffer.data(),
                    sourceBuffer.data(),
                    0.0f,
                    1.0f,
                    itemsToWrite);
            m_firstInputChunk = false;
            return itemsToWrite;
        } else {
            return copyRing(sourceBuffer,
                    0,
                    mixxx::spanutil::spanFromPtrLen(m_buffer.data(), m_buffer.size()),
                    m_writePos,
                    itemsToWrite);
        }
    }();

    // Calculate the new write position. If the new write position
    // is after the ring buffer end, move it around from the start
    // of the ring buffer.
    m_writePos = (m_writePos + numItems);

    if (m_writePos >= m_buffer.size()) {
        m_writePos = m_writePos - m_buffer.size();
    }

    return numItems;
}
