#include "util/singularsamplebuffer.h"

#include "util/sample.h"

#define DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer \
    DEBUG_ASSERT(0 <= m_headOffset); \
    DEBUG_ASSERT(m_headOffset <= m_tailOffset); \
    DEBUG_ASSERT(m_tailOffset <= m_primaryBuffer.size()); \
    DEBUG_ASSERT(!isEmpty() || (0 == m_headOffset)); \
    DEBUG_ASSERT(!isEmpty() || (0 == m_tailOffset))

SingularSampleBuffer::SingularSampleBuffer()
        : m_headOffset(0),
          m_tailOffset(0) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
}

SingularSampleBuffer::SingularSampleBuffer(SINT capacity)
    : m_primaryBuffer(capacity),
      m_headOffset(0),
      m_tailOffset(0) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
}

void SingularSampleBuffer::reset() {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    m_headOffset = 0;
    m_tailOffset = 0;

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
}

void SingularSampleBuffer::resetCapacity(SINT capacity) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    if (m_primaryBuffer.size() != capacity) {
        SampleBuffer(capacity).swap(m_primaryBuffer);
    }
    resetOffsets();

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
}

void SingularSampleBuffer::swapBuffers(SampleBuffer& secondaryBuffer) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    DEBUG_ASSERT(m_primaryBuffer.size() == secondaryBuffer.size());

    // SampleUtil::copy() requires that the source and destination
    // memory regions are disjunct. Double-buffering is necessary
    // to satisfy this precondition.
    SampleUtil::copy(
            secondaryBuffer.data(),
            m_primaryBuffer.data(m_headOffset),
            getSize());
    m_primaryBuffer.swap(secondaryBuffer);
    // shift offsets
    m_tailOffset -= m_headOffset;
    m_headOffset = 0;

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
}

void SingularSampleBuffer::trim(SampleBuffer& secondaryBuffer) {
    if (0 < m_headOffset) {
        if (isEmpty()) {
            reset();
        } else {
            swapBuffers(secondaryBuffer);
        }
    }
}

SampleBuffer::WritableChunk SingularSampleBuffer::writeToTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    const SINT tailLength = math_min(size, getTailCapacity());
    const SampleBuffer::WritableChunk tailChunk(
            m_primaryBuffer, m_tailOffset, tailLength);
    m_tailOffset += tailLength;

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    return tailChunk;
}

SampleBuffer::ReadableChunk SingularSampleBuffer::readFromTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    const SINT tailLength = math_min(size, getSize());
    m_tailOffset -= tailLength;
    const SampleBuffer::ReadableChunk tailChunk(
            m_primaryBuffer, m_tailOffset, tailLength);
    if (isEmpty()) {
        // Internal buffer becomes empty and can safely be reset
        // to extend the tail capacity for future growth
        resetOffsets();
    }

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    return tailChunk;
}

SampleBuffer::ReadableChunk SingularSampleBuffer::readFromHead(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    const SINT headLength = math_min(size, getSize());
    const SampleBuffer::ReadableChunk headChunk(
            m_primaryBuffer, m_headOffset, headLength);
    m_headOffset += headLength;
    if (isEmpty()) {
        // Internal buffer becomes empty and can safely be reset
        // to extend the tail capacity for future growth
        resetOffsets();
    }

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    return headChunk;
}
