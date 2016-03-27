#ifndef MIXXX_UTIL_CIRCULARSAMPLEBUFFER_H
#define MIXXX_UTIL_CIRCULARSAMPLEBUFFER_H

#include "util/singularsamplebuffer.h"

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
    CircularSampleBuffer(const CircularSampleBuffer&) = delete;
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

#endif // MIXXX_UTIL_CIRCULARSAMPLEBUFFER_H
