#include "singularsamplebuffer.h"

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
