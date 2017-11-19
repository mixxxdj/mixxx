#pragma once


#include "util/indexrange.h"
#include "util/samplebuffer.h"


namespace mixxx {

// A FIFO/LIFO sample buffer with fixed capacity and range checking.
// It works best when consuming all buffered samples before writing
// any new samples.
//
// This class is not thread-safe and is not intended to be used from
// multiple threads!
class ReadAheadSampleBuffer final {
  public:
    explicit ReadAheadSampleBuffer(
            SINT capacity = 0);
    ReadAheadSampleBuffer(
            const ReadAheadSampleBuffer& that)
        : ReadAheadSampleBuffer(that, that.capacity()) {
    }
    ReadAheadSampleBuffer(
            ReadAheadSampleBuffer&&) = default;

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

    // Tries to adjust the capacity taking into account the
    // current contents of the buffer. The resulting capacity
    // may therefore be higher than requested when shrinking
    // the buffer.
    void adjustCapacity(SINT capacity);

    // Discards all buffered samples.
    void clear();

    bool empty() const {
        return m_readableRange.empty();
    }

    // The number of samples that could be written instantly without
    // internal reorganization, i.e. the remaining capacity of the
    // buffer.
    SINT writableLength() const {
        return capacity() - m_readableRange.end();
    }

    // Reserves space at the buffer's back end for writing samples.
    //
    // Returns a pointer to the continuous memory region and the
    // actual number of samples that have been reserved. The maximum
    // length is limited by writableLength().
    //
    // The returned pointer is valid until the next write() operation.
    SampleBuffer::WritableSlice write(SINT writeLength);

    // The number of readable samples.
    SINT readableLength() const {
        return m_readableRange.length();
    }

    // Consumes buffered samples in FIFO order.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of readable samples. The maximum length is limited by
    // readableLength().
    //
    // The returned pointer is valid until the next write() operation.
    SampleBuffer::ReadableSlice readFifo(SINT readLength);

    // Consumes buffered samples in LIFO order.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of readable samples. The maximum length is limited by
    // readableLength().
    //
    // The returned pointer is valid until the next write() operation.
    SampleBuffer::ReadableSlice readLifo(SINT readLength);

  private:
    ReadAheadSampleBuffer(
            const ReadAheadSampleBuffer& that,
            SINT capacity);

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
