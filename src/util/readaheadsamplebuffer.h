#ifndef MIXXX_UTIL_READAHEADSAMPLEBUFFER_H
#define MIXXX_UTIL_READAHEADSAMPLEBUFFER_H

#include "util/indexrange.h"
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
    explicit ReadAheadSampleBuffer(
            SINT capacity);
    ReadAheadSampleBuffer(
            const ReadAheadSampleBuffer& that)
        : ReadAheadSampleBuffer(that, that.capacity()) {
    }
    ReadAheadSampleBuffer(
            const ReadAheadSampleBuffer& that,
            SINT capacity);
    ReadAheadSampleBuffer(
            ReadAheadSampleBuffer&&) = default;
    virtual ~ReadAheadSampleBuffer() {}

    ReadAheadSampleBuffer& operator=(
            const ReadAheadSampleBuffer& that) {
        *this = ReadAheadSampleBuffer(that); // copy ctor + move assignment
        return *this;
    }
    ReadAheadSampleBuffer& operator=(
            ReadAheadSampleBuffer&& that) {
        swap(that);
        return *this;
    }
    
    void swap(
            ReadAheadSampleBuffer& that);

    // The maximum capacity of the buffer.
    SINT capacity() const {
        return m_sampleBuffer.size();
    }

    // The number of samples that could be written instantly,
    // i.e. the remaining capacity of the buffer.
    SINT writableLength() const {
        return capacity() - m_readableRange.end();
    }

    // The number of readable samples.
    SINT readableLength() const {
        return m_readableRange.length();
    }

    bool empty() const {
        return m_readableRange.empty();
    }

    // Discards all buffered samples.
    void clear();

    // Reserves space at the buffer's tail for writing samples. The internal
    // buffer is trimmed if necessary to provide a continuous memory region.
    //
    // Returns a pointer to the continuous memory region and the actual number
    // of samples that have been reserved. The maximum growth is limited by
    // getTailCapacity() and might be increased by calling trim().
    SampleBuffer::WritableSlice writeToTail(
            SINT writeLength);

    // Shrinks the buffer from the tail for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    SampleBuffer::ReadableSlice readFromTail(
            SINT readLength);

    // Shrinks the buffer from the head for reading buffered samples.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of buffered samples that have been dropped. The pointer is
    // valid for reading as long as no modifying member function is called!
    SampleBuffer::ReadableSlice readFromHead(
            SINT readLength);

  private:
    void adjustReadableRange() {
        if (m_readableRange.empty()) {
            m_readableRange = IndexRange::between(0, 0);
        }
    }

    SampleBuffer m_sampleBuffer;
    IndexRange m_readableRange;
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
