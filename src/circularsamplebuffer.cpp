#include <circularsamplebuffer.h>
#include "sampleutil.h"

#define DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer \
    DEBUG_ASSERT(m_primaryBuffer.size() == m_secondaryBuffer.size()); \
    DEBUG_ASSERT(0 <= m_headOffset); \
    DEBUG_ASSERT(m_headOffset <= m_tailOffset); \
    DEBUG_ASSERT(m_tailOffset <= m_primaryBuffer.size()); \
    DEBUG_ASSERT(!isEmpty() || (0 == m_headOffset)); \
    DEBUG_ASSERT(!isEmpty() || (0 == m_tailOffset))

CircularSampleBuffer::CircularSampleBuffer()
        : m_headOffset(0),
          m_tailOffset(0) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

CircularSampleBuffer::CircularSampleBuffer(SINT capacity)
    : m_primaryBuffer(capacity),
      m_secondaryBuffer(capacity),
      m_headOffset(0),
      m_tailOffset(0) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

void CircularSampleBuffer::swap(CircularSampleBuffer& other) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    std::swap(m_primaryBuffer, other.m_primaryBuffer);
    std::swap(m_secondaryBuffer, other.m_secondaryBuffer);
    std::swap(m_headOffset, other.m_headOffset);
    std::swap(m_tailOffset, other.m_tailOffset);

    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

void CircularSampleBuffer::swapBuffers() {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    // SampleUtil::copy() requires that the source and destination
    // memory regions are disjunct. Double-buffering is necessary
    // to satisfy this precondition.
    SampleUtil::copy(
            m_secondaryBuffer.data(),
            m_primaryBuffer.data() + m_headOffset,
            getSize());
    m_primaryBuffer.swap(m_secondaryBuffer);
    // shift offsets
    m_tailOffset -= m_headOffset;
    m_headOffset = 0;

    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

void CircularSampleBuffer::reset() {
    m_headOffset = 0;
    m_tailOffset = 0;
}

void CircularSampleBuffer::trim() {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    if (0 < m_headOffset) {
        if (isEmpty()) {
            reset();
        } else {
            swapBuffers();
        }
    }

    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

std::pair<CSAMPLE*, SINT> CircularSampleBuffer::growTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    CSAMPLE* const tailData = m_primaryBuffer.data() + m_tailOffset;
    const SINT tailSize = math_min(size, getTailCapacity());
    m_tailOffset += tailSize;

    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
    return std::make_pair(tailData, tailSize);
}

std::pair<const CSAMPLE*, SINT> CircularSampleBuffer::shrinkTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    const SINT tailSize = math_min(size, getSize());
    m_tailOffset -= tailSize;
    const CSAMPLE* const tailData = m_primaryBuffer.data() + m_tailOffset;
    if (tailSize == getSize()) {
        DEBUG_ASSERT(isEmpty());
        reset();
    }

    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
    return std::make_pair(tailData, tailSize);
}

std::pair<const CSAMPLE*, SINT> CircularSampleBuffer::shrinkHead(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    const CSAMPLE* const headData = m_primaryBuffer.data() + m_headOffset;
    const SINT headSize = math_min(size, getSize());
    if (headSize == getSize()) {
        // buffer becomes empty
        reset();
    } else {
        m_headOffset += headSize;
    }

    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
    return std::make_pair(headData, headSize);
}
