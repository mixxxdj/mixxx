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
    : ReadAheadSampleBuffer(0) {
    
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
}

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
    m_readableRange.growBackRange(that.readableLength());
    
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

SampleBuffer::WritableSlice ReadAheadSampleBuffer::write(SINT writeLength) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT tailLength = math_min(writeLength, writableLength());
    const SampleBuffer::WritableSlice tailSlice(
            m_sampleBuffer, m_readableRange.end(), tailLength);
    m_readableRange.growBackRange(tailLength);

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return tailSlice;
}

SampleBuffer::ReadableSlice ReadAheadSampleBuffer::readFifo(SINT readLength) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT tailLength = math_min(readLength, readableLength());
    m_readableRange.dropBackRange(tailLength);
    const SampleBuffer::ReadableSlice tailSlice(
            m_sampleBuffer, m_readableRange.end(), tailLength);
    adjustReadableRange();

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return tailSlice;
}

SampleBuffer::ReadableSlice ReadAheadSampleBuffer::readLifo(SINT readLength) {
    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;

    const SINT headLength = math_min(readLength, readableLength());
    const SampleBuffer::ReadableSlice headSlice(
            m_sampleBuffer, m_readableRange.start(), headLength);
    m_readableRange.dropFrontRange(headLength);
    adjustReadableRange();

    DEBUG_ASSERT_CLASS_INVARIANT_ReadAheadSampleBuffer;
    return headSlice;
}

} // namespace mixxx
