#include "util/circularsamplebuffer.h"

CircularSampleBuffer::CircularSampleBuffer(SINT capacity)
    : SingularSampleBuffer(capacity),
      m_secondaryBuffer(capacity) {
}

void CircularSampleBuffer::resetCapacity(SINT capacity) {
    if (m_secondaryBuffer.size() != capacity) {
        SampleBuffer secondaryBuffer(capacity); // may throw
        SingularSampleBuffer::resetCapacity(capacity); // may throw
        secondaryBuffer.swap(m_secondaryBuffer); // does not throw
    } else {
        reset();
    }
}
