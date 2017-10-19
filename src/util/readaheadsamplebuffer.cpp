#include "util/readaheadsamplebuffer.h"

#include "util/sample.h"


#define DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer \
    DEBUG_ASSERT(m_readableRange.orientation() != IndexRange::Orientation::Backward); \
    DEBUG_ASSERT(0 <= m_readableRange.start()); \
    DEBUG_ASSERT(m_readableRange.end() <= m_sampleBuffer.size()); \
    DEBUG_ASSERT(!empty() || (0 == m_readableRange.start())); \
    DEBUG_ASSERT(!empty() || (0 == m_readableRange.end()))


namespace mixxx {

ReadAheadSampleBuffer::ReadAheadSampleBuffer()
    : ReadAheadSampleBuffer(0),
      m_readableRange(IndexRange::between(0, 0)) {
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

ReadAheadSampleBuffer::ReadAheadSampleBuffer(
        SINT capacity)
    : m_sampleBuffer(capacity),
      m_headOffset(0),
      m_tailOffset(0) {
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

ReadAheadSampleBuffer::ReadAheadSampleBuffer(
        const ReadAheadSampleBuffer& that,
        SINT capacity) 
    : ReadAheadSampleBuffer(capacity) {
    SampleUtil::copy(
        m_sampleBuffer.data(),
        that.m_sampleBuffer.data(that.m_headOffset),
        that.getSize());
    m_tailOffset += that.getSize();
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

ReadAheadSampleBuffer::ReadAheadSampleBuffer(ReadAheadSampleBuffer&& that)
    : m_sampleBuffer(std::move(that.m_sampleBuffer)),
      m_headOffset(that.m_headOffset),
      m_tailOffset(that.m_tailOffset) {
    that.m_headOffset = 0;
    that.m_tailOffset = 0;
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

void ReadAheadSampleBuffer::swap(ReadAheadSampleBuffer& that) {
    m_sampleBuffer.swap(that.m_sampleBuffer);
    std::swap(m_headOffset, that.m_headOffset);
    std::swap(m_tailOffset, that.m_tailOffset);
}

void ReadAheadSampleBuffer::reset() {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    m_headOffset = 0;
    m_tailOffset = 0;

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

void ReadAheadSampleBuffer::resetCapacity(SINT capacity) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    if (m_sampleBuffer.size() != capacity) {
        SampleBuffer(capacity).swap(m_sampleBuffer);
    }
    resetOffsets();

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

SampleBuffer::WritableSlice ReadAheadSampleBuffer::writeToTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT tailLength = math_min(size, getTailCapacity());
    const SampleBuffer::WritableSlice tailSlice(
            m_sampleBuffer, m_tailOffset, tailLength);
    m_tailOffset += tailLength;

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return tailSlice;
}

SampleBuffer::ReadableSlice ReadAheadSampleBuffer::readFromTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT tailLength = math_min(size, getSize());
    m_tailOffset -= tailLength;
    const SampleBuffer::ReadableSlice tailSlice(
            m_sampleBuffer, m_tailOffset, tailLength);
    if (isEmpty()) {
        // Internal buffer becomes empty and can safely be reset
        // to extend the tail capacity for future growth
        resetOffsets();
    }

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return tailSlice;
}

SampleBuffer::ReadableSlice ReadAheadSampleBuffer::readFromHead(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT headLength = math_min(size, getSize());
    const SampleBuffer::ReadableSlice headSlice(
            m_sampleBuffer, m_headOffset, headLength);
    m_headOffset += headLength;
    if (isEmpty()) {
        // Internal buffer becomes empty and can safely be reset
        // to extend the tail capacity for future growth
        resetOffsets();
    }

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return headSlice;
}

} // namespace mixxx
