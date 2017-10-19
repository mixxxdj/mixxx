#ifndef MIXXX_UTIL_READAHEADSAMPLEBUFFER_H
#define MIXXX_UTIL_READAHEADSAMPLEBUFFER_H

#include "util/samplebuffer.h"


namespace mixxx {

// A FIFO/LIFO sample buffer with fixed capacity and range checking.
// It works best when consuming all buffered samples before writing
// any new samples.
//
// This class is not thread-safe and is not intended to be used from
// multiple threads!
class ReadAheadSampleBuffer {
  public:
    ReadAheadSampleBuffer();
    explicit ReadAheadSampleBuffer(SINT capacity);
    ReadAheadSampleBuffer(const ReadAheadSampleBuffer& that)
        : ReadAheadSampleBuffer(that, that.getCapacity()) {
    }
    ReadAheadSampleBuffer(const ReadAheadSampleBuffer& that, SINT capacity);
    ReadAheadSampleBuffer(ReadAheadSampleBuffer&&);
    virtual ~ReadAheadSampleBuffer() {}

    ReadAheadSampleBuffer& operator=(const ReadAheadSampleBuffer&) = delete;
    ReadAheadSampleBuffer& operator=(ReadAheadSampleBuffer&&) = delete;
    
    void swap(ReadAheadSampleBuffer& that);

    // The initial/total capacity of the buffer.
    SINT getCapacity() const {
        return m_sampleBuffer.size();
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
    SampleBuffer::WritableSlice writeToTail(SINT size);

    // Shrinks the buffer from the tail for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    SampleBuffer::ReadableSlice readFromTail(SINT size);

    // Shrinks the buffer from the head for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    SampleBuffer::ReadableSlice readFromHead(SINT size);

  private:
    void resetOffsets() {
        m_headOffset = 0;
        m_tailOffset = 0;
    }

    SampleBuffer m_sampleBuffer;
    SINT m_headOffset;
    SINT m_tailOffset;
};

} // namespace mixxx

namespace std {
    
// Template specialization of std::swap() for ReadAheadSampleBuffer
template<>
inline void swap(::mixxx::ReadAheadSampleBuffer& lhs, ::mixxx::ReadAheadSampleBuffer& rhs) {
    lhs.swap(rhs);
}

}  // namespace std

#endif // MIXXX_UTIL_READAHEADSAMPLEBUFFER_H
