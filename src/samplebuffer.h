#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include "util/types.h" // CSAMPLE

#include <algorithm> // std::swap

// A sample buffer with properly aligned memory to enable SSE optimizations.
//
// No resize operation is provided intentionally for maximum efficiency!
// If the size of an existing sample buffer needs to be altered after
// construction this can simply be achieved by swapping the contents with
// a temporary sample buffer that has been constructed with the appropriate
// size:
//
//     SampleBuffer sampleBuffer(oldSize);
//     ...
//     SampleBuffer(newSize).swap(sampleBuffer);
//     // Now the data in sampleBuffer is uninitialized and
//     // sampleBuffer.size() == newSize
//
// No sample data is copied when swapping the contents of two samples buffers
// for resizing!
class SampleBuffer {
    Q_DISABLE_COPY(SampleBuffer);

  public:
    typedef size_t size_type;
    typedef CSAMPLE value_type;

    // random access iterators
    typedef value_type* iterator;
    typedef const value_type* const_iterator;

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

    value_type* data() {
        return m_data;
    }
    const value_type* data() const {
        return m_data;
    }

    value_type& operator[](size_type index) {
        return m_data[index];
    }
    value_type operator[](size_type index) const {
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
    value_type* m_data;
    size_type m_size;
};

#endif // SAMPLEBUFFER_H
