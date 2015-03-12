#include "circularsamplebuffer.h"

#include "sampleutil.h"

CircularSampleBuffer::CircularSampleBuffer() {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

CircularSampleBuffer::CircularSampleBuffer(SINT capacity)
    : SingularSampleBuffer(capacity),
      m_secondaryBuffer(capacity) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;
}

void CircularSampleBuffer::resetCapacity(SINT capacity) {
    DEBUG_ASSERT_CLASS_INVARIANT_CircularSampleBuffer;

    if (m_secondaryBuffer.size() != capacity) {
        SampleBuffer secondaryBuffer(capacity);
        SingularSampleBuffer::resetCapacity(capacity);
        secondaryBuffer.swap(m_secondaryBuffer);
    } else {
        reset();
    }

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
