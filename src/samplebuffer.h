#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include "util/types.h" // CSAMPLE

#include <algorithm> // std::swap

// A sample buffer with properly aligned memory to enable SSE optimizations.
// The public interface closely follows that of std::vector.
//
// No resize operation is provided intentionally for maximum efficiency!
// If the size of an existing sample buffer needs to be altered after
// construction this can simply be achieved by swapping the contents with
// a temporary sample buffer that has been constructed with the desired
// size:
//
//     SampleBuffer sampleBuffer(oldSize);
//     ...
//     SampleBuffer tempBuffer(newSize)
//     ... copy data from sampleBuffer to tempBuffer...
//     sampleBuffer.swap(sampleBuffer);
//
// The local variable tempBuffer can be omitted if no data needs to be
// copied from the existing sampleBuffer:
//
//     SampleBuffer sampleBuffer(oldSize);
//     ...
//     SampleBuffer(newSize).swap(sampleBuffer);
//
// After construction the content of the buffer is uninitialized.
class SampleBuffer {
    Q_DISABLE_COPY(SampleBuffer);

  public:
    typedef size_t size_type;
    typedef CSAMPLE value_type;

    typedef value_type& reference;
    typedef const value_type& const_reference;

    typedef value_type* pointer;
    typedef const value_type* const_pointer;

    // random access iterators
    typedef pointer iterator;
    typedef const_pointer const_iterator;

    SampleBuffer():
        m_data(NULL),
        m_size(0) {
    }
    explicit SampleBuffer(size_type size);
    virtual ~SampleBuffer();

    void swap(SampleBuffer& other) {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
    }

    size_type size() const {
        return m_size;
    }

    pointer data() {
        return m_data;
    }
    const_pointer data() const {
        return m_data;
    }

    reference operator[](size_type index) {
        return m_data[index];
    }
    const_reference operator[](size_type index) const {
        return m_data[index];
    }

    iterator begin() {
        return m_data;
    }
    const_iterator begin() const {
        return m_data;
    }

    iterator end() {
        return m_data + m_size;
    }
    const_iterator end() const {
        return m_data + m_size;
    }

  private:
    pointer m_data;
    size_type m_size;
};

#endif // SAMPLEBUFFER_H
