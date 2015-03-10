#ifndef FIFOSAMPLEBUFFER_H
#define FIFOSAMPLEBUFFER_H

#include "samplebuffer.h"

// A FIFO sample buffer with a fixed capacity, range checking,
// and double-buffering.
//
// Maximum performance is achieved when consuming all buffered
// samples before the capacity is exhausted. This use case does
// not require any internal copying/moving of buffered samples.
class FifoSampleBuffer {
    Q_DISABLE_COPY(FifoSampleBuffer);

public:
    FifoSampleBuffer();
    explicit FifoSampleBuffer(SINT capacity);

    void swap(FifoSampleBuffer& other);

    bool isEmpty() const {
        return m_writeOffset <= m_readOffset;
    }

    // Returns the current size of the buffer, i.e. the number of
    // buffered samples that have been written and are available
    // for reading.
    SINT getSize() const {
        return m_writeOffset - m_readOffset;
    }

    // Discards all buffered samples and resets the buffer to its
    // initial state.
    void reset();

    // Moves all buffered samples to the beginning of the internal buffer.
    //
    // This function will be called implicitly by growTail() when needed.
    void trim();

    // Reserves space at the buffer's tail for writing samples. The internal
    // buffer is trimmed if necessary to provide a continuous memory region.
    //
    // Returns a pointer to the continuous memory region and the actual number
    // of samples that have been reserved. The pointer is valid for writing as
    // long as no modifying member function is called!
    std::pair<CSAMPLE*, SINT> growTail(SINT size);

    // Shrinks the buffer from the tail discarding the buffered samples.
    //
    // Returns the actual number of buffered samples that have been discarded.
    SINT shrinkTail(SINT size);

    // Shrinks the buffer from the head for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    std::pair<const CSAMPLE*, SINT> shrinkHead(SINT size);

private:
    void swapBuffers();

    SampleBuffer m_sampleBuffer;
    SampleBuffer m_shadowBuffer;
    SINT m_readOffset;
    SINT m_writeOffset;
};

namespace std
{

// Template specialization of std::swap for FifoSampleBuffer.
template<>
inline void swap(FifoSampleBuffer& lhs, FifoSampleBuffer& rhs) {
    lhs.swap(rhs);
}

}

#endif // FIFOSAMPLEBUFFER_H
