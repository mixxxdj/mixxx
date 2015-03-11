#ifndef FIFOSAMPLEBUFFER_H
#define FIFOSAMPLEBUFFER_H

#include "samplebuffer.h"

// A circular FIFO/LIFO sample buffer with fixed capacity, range checking,
// and double-buffering.
//
// Maximum performance is achieved when consuming all buffered samples
// before the capacity is exhausted. This use case does not require any
// internal copying/moving of buffered samples. Otherwise the buffer
// needs to be trimmed when running out of tail capacity.
//
// This class is not thread-safe and not intended to be used from multiple
// threads!
class CircularSampleBuffer {
    Q_DISABLE_COPY(CircularSampleBuffer);

public:
    CircularSampleBuffer();
    explicit CircularSampleBuffer(SINT capacity);

    void swap(CircularSampleBuffer& other);

    // The initial/total capacity of the buffer.
    SINT getCapacity() const {
        return m_primaryBuffer.size();
    }

    // The capacity at the tail that is immediately available
    // without trimming the buffer.
    SINT getTailCapacity() const {
        return getCapacity() - m_tailOffset;
    }

    bool isEmpty() const {
        return m_tailOffset <= m_headOffset;
    }

    // Returns the current size of the buffer, i.e. the number of
    // buffered samples that have been written and are available
    // for reading.
    SINT getSize() const {
        return m_tailOffset - m_headOffset;
    }

    // Discards all buffered samples and resets the buffer to its
    // initial state.
    void reset();

    // Moves all buffered samples to the beginning of the internal buffer.
    //
    // This will increase the free capacity at the tail returned by
    // getTailCapacity() to the maximum amount getCapacity() - getSize().
    void trim();

    // Reserves space at the buffer's tail for writing samples. The internal
    // buffer is trimmed if necessary to provide a continuous memory region.
    //
    // Returns a pointer to the continuous memory region and the actual number
    // of samples that have been reserved. The maximum growth is limited by
    // getTailCapacity() and might be increased by calling trim().
    std::pair<CSAMPLE*, SINT> growTail(SINT size);

    // Shrinks the buffer from the tail for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    std::pair<const CSAMPLE*, SINT> shrinkTail(SINT size);

    // Shrinks the buffer from the head for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    std::pair<const CSAMPLE*, SINT> shrinkHead(SINT size);

private:
    void swapBuffers();

    SampleBuffer m_primaryBuffer;
    SampleBuffer m_secondaryBuffer;
    SINT m_headOffset;
    SINT m_tailOffset;
};

namespace std
{

// Template specialization of std::swap for CircularSampleBuffer.
template<>
inline void swap(CircularSampleBuffer& lhs, CircularSampleBuffer& rhs) {
    lhs.swap(rhs);
}

}

#endif // FIFOSAMPLEBUFFER_H
