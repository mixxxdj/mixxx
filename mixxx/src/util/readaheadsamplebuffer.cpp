#include "util/readaheadsamplebuffer.h"

#include "util/sample.h"


#define DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer \
    DEBUG_ASSERT(m_readableRange.orientation() != IndexRange::Orientation::Backward); \
    DEBUG_ASSERT(0 <= m_readableRange.start()); \
    DEBUG_ASSERT(m_readableRange.end() <= m_sampleBuffer.size()); \
    DEBUG_ASSERT(!empty() || (0 == m_readableRange.start())); \
    DEBUG_ASSERT(!empty() || (0 == m_readableRange.end()))


namespace mixxx {

ReadAheadSampleBuffer::ReadAheadSampleBuffer(
        SINT capacity)
    : m_sampleBuffer(capacity),
      m_readableRange(IndexRange::between(0, 0)) {
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

ReadAheadSampleBuffer::ReadAheadSampleBuffer(
        const ReadAheadSampleBuffer& that,
        SINT capacity)
    : ReadAheadSampleBuffer(capacity) {
    DEBUG_ASSERT(that.readableLength() <= capacity);
    // Copy all readable contents to the beginning of the buffer
    // for maximizing the writable capacity.
    SampleUtil::copy(
        m_sampleBuffer.data(),
        that.m_sampleBuffer.data(that.m_readableRange.start()),
        that.readableLength());
    m_readableRange.growBack(that.readableLength());
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

void ReadAheadSampleBuffer::swap(ReadAheadSampleBuffer& that) {
    m_sampleBuffer.swap(that.m_sampleBuffer);
    std::swap(m_readableRange, that.m_readableRange);
}

void ReadAheadSampleBuffer::clear() {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    m_readableRange = IndexRange::between(0, 0);

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

void ReadAheadSampleBuffer::adjustCapacity(SINT capacity) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    SINT newCapacity = std::max(readableLength(), capacity);
    if (newCapacity != this->capacity()) {
        ReadAheadSampleBuffer tmp(*this, newCapacity);
        swap(tmp);
    }

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

SampleBuffer::WritableSlice ReadAheadSampleBuffer::growForWriting(SINT maxWriteLength) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT tailLength = std::min(maxWriteLength, writableLength());
    const SampleBuffer::WritableSlice tailSlice(
            m_sampleBuffer, m_readableRange.end(), tailLength);
    m_readableRange.growBack(tailLength);

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return tailSlice;
}

SINT ReadAheadSampleBuffer::shrinkAfterWriting(SINT maxShrinkLength) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT shrinkLength = std::min(maxShrinkLength, readableLength());
    m_readableRange.shrinkBack(shrinkLength);
    // If the buffer has become empty reset the write head back to the start
    // of the available memory
    if (m_readableRange.empty()) {
        m_readableRange = IndexRange::between(0, 0);
    }

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return shrinkLength;
}

SampleBuffer::ReadableSlice ReadAheadSampleBuffer::shrinkForReading(SINT maxReadLength) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT headLength = std::min(maxReadLength, readableLength());
    const SampleBuffer::ReadableSlice headSlice(
            m_sampleBuffer, m_readableRange.start(), headLength);
    m_readableRange.shrinkFront(headLength);
    // If the buffer has become empty reset the write head back to the start
    // of the available memory
    if (m_readableRange.empty()) {
        m_readableRange = IndexRange::between(0, 0);
    }

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return headSlice;
}

} // namespace mixxx
