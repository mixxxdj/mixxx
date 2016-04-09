#ifndef MIXXX_UTIL_SINGULARSAMPLEBUFFER_H
#define MIXXX_UTIL_SINGULARSAMPLEBUFFER_H

#include "util/samplebuffer.h"

// A singular FIFO/LIFO sample buffer with fixed capacity and range
// checking.
//
// Common use case: Consume all buffered samples before the capacity
// is exhausted.
//
// This class is not thread-safe and not intended to be used from multiple
// threads!
class SingularSampleBuffer {
  public:
    SingularSampleBuffer();
    explicit SingularSampleBuffer(SINT capacity);
    SingularSampleBuffer(const SingularSampleBuffer&) = delete;
    virtual ~SingularSampleBuffer() {}

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

    // Discards all buffered samples and resets the buffer to its
    // initial state with a new capacity
    virtual void resetCapacity(SINT capacity);

    // Reserves space at the buffer's tail for writing samples. The internal
    // buffer is trimmed if necessary to provide a continuous memory region.
    //
    // Returns a pointer to the continuous memory region and the actual number
    // of samples that have been reserved. The maximum growth is limited by
    // getTailCapacity() and might be increased by calling trim().
    SampleBuffer::WritableChunk writeToTail(SINT size);

    // Shrinks the buffer from the tail for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    SampleBuffer::ReadableChunk readFromTail(SINT size);

    // Shrinks the buffer from the head for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    SampleBuffer::ReadableChunk readFromHead(SINT size);

  protected:
    void trim(SampleBuffer& secondaryBuffer);

  private:
    void swapBuffers(SampleBuffer& secondaryBuffer);

    void resetOffsets() {
        m_headOffset = 0;
        m_tailOffset = 0;
    }

    SampleBuffer m_primaryBuffer;
    SINT m_headOffset;
    SINT m_tailOffset;
};

#endif // MIXXX_UTIL_SINGULARSAMPLEBUFFER_H
