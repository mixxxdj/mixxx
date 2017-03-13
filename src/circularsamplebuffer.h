#ifndef CIRCULARSAMPLEBUFFER_H
#define CIRCULARSAMPLEBUFFER_H

#include "singularsamplebuffer.h"

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
class CircularSampleBuffer: public SingularSampleBuffer {
public:
    CircularSampleBuffer() {}
    explicit CircularSampleBuffer(SINT capacity);

    void resetCapacity(SINT capacity) override;

    // Moves all buffered samples to the beginning of the internal buffer.
    //
    // This will increase the free capacity at the tail returned by
    // getTailCapacity() to the maximum amount getCapacity() - getSize().
    void trim() {
        SingularSampleBuffer::trim(m_secondaryBuffer);
    }

private:
    SampleBuffer m_secondaryBuffer;
};

#endif // CIRCULARSAMPLEBUFFER_H
