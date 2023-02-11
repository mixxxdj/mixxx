#pragma once


#include "util/indexrange.h"
#include "util/samplebuffer.h"


namespace mixxx {

// A FIFO sample buffer with fixed capacity and range checking.
//
// Samples are written at the tail and read from the head (FIFO).
// It is intended to consume all buffered samples before writing
// any new samples. A full featured ring buffer is not needed for
// this purpose.
//
// The API is not designed for concurrent readers and writers!
// Samples reserved for writing are immediately available for
// reading, even if the writer has not not yet written any samples.
// With this in mind the implementation does not make any attempts
// to be thread-safe!
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

    // The capacity is limited by the size of the underlying buffer.
    constexpr SINT capacity() const {
        return m_sampleBuffer.size();
    }

    // Tries to adjust the capacity taking into account the
    // current contents of the buffer. The resulting capacity
    // may therefore be higher than requested when trying to
    // reduce the current capacity.
    void adjustCapacity(SINT capacity);

    // Discards all buffered samples.
    void clear();

    bool empty() const {
        return m_readableRange.empty();
    }

    // The number of readable samples.
    SINT readableLength() const {
        return m_readableRange.length();
    }

    // The number of samples that could be written instantly without
    // internal reorganization, i.e. the remaining capacity of the
    // buffer.
    // Only the space between the end of the slice occupied by
    // written (= readable) samples and the end of the allocated
    // buffer is available for writing!
    SINT writableLength() const {
        return capacity() - m_readableRange.end();
    }

    // Reserves space for writing samples by growing the readable
    // range at the back towards the end of the underlying buffer.
    //
    // Returns a pointer to the continuous memory region and the
    // actual number of samples that have been reserved as a slice.
    // The maximum length is limited by writableLength() and the
    // returned slice might be shorter than requested.
    //
    // The returned slice is only valid until the next non-const
    // operation on this buffer.
    SampleBuffer::WritableSlice growForWriting(SINT maxWriteLength);

    // Discards the last samples that have been written by shrinking
    // the readable range from the back.
    //
    // Returns the number of samples that have actually been discarded.
    // The number of samples that can be discarded is limited by
    // readableLength() and the returned length might be shorter than
    // requested.
    SINT shrinkAfterWriting(SINT maxShrinkLength);

    // Consumes buffered samples from the front of the readable range
    // thereby shrinking it.
    //
    // Returns a pointer to the continuous memory region and the actual
    // number of readable samples as a slice. The maximum length is
    // limited by readableLength() and the returned slice might be
    // shorter than requested.
    //
    // The returned slice is only valid until the next non-const
    // operation on this buffer.
    SampleBuffer::ReadableSlice shrinkForReading(SINT maxReadLength);

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
