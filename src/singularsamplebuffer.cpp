#include "singularsamplebuffer.h"

#include "sampleutil.h"

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
    reset();

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
            m_primaryBuffer.data() + m_headOffset,
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

std::pair<CSAMPLE*, SINT> SingularSampleBuffer::growTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    CSAMPLE* const tailData = m_primaryBuffer.data() + m_tailOffset;
    const SINT tailSize = math_min(size, getTailCapacity());
    m_tailOffset += tailSize;

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    return std::make_pair(tailData, tailSize);
}

std::pair<const CSAMPLE*, SINT> SingularSampleBuffer::shrinkTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    const SINT tailSize = math_min(size, getSize());
    m_tailOffset -= tailSize;
    const CSAMPLE* const tailData = m_primaryBuffer.data() + m_tailOffset;
    if (tailSize == getSize()) {
        DEBUG_ASSERT(isEmpty());
        reset();
    }

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    return std::make_pair(tailData, tailSize);
}

std::pair<const CSAMPLE*, SINT> SingularSampleBuffer::shrinkHead(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;

    const CSAMPLE* const headData = m_primaryBuffer.data() + m_headOffset;
    const SINT headSize = math_min(size, getSize());
    if (headSize == getSize()) {
        // buffer becomes empty
        reset();
    } else {
        m_headOffset += headSize;
    }

    DEBUG_ASSERT_CLASS_INVARIANT_SingularSampleBuffer;
    return std::make_pair(headData, headSize);
}
