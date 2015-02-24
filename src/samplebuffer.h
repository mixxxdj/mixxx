#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

<<<<<<< HEAD
#include "util/types.h"
=======
#include "util/types.h" // CSAMPLE
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations

#include <algorithm> // std::swap

// A sample buffer with properly aligned memory to enable SSE optimizations.
<<<<<<< HEAD
<<<<<<< HEAD
// After construction the content of the buffer is uninitialized. No resize
// operation is provided intentionally because malloc might block!
//
// Hint: If the size of an existing sample buffer ever needs to be altered
// after construction this can simply be achieved by swapping the contents
// with a temporary sample buffer that has been constructed with the desired
=======
=======
// The public interface closely follows that of std::vector.
>>>>>>> Align SampleBuffer with std::vector
//
// No resize operation is provided intentionally for maximum efficiency!
// If the size of an existing sample buffer needs to be altered after
// construction this can simply be achieved by swapping the contents with
<<<<<<< HEAD
// a temporary sample buffer that has been constructed with the appropriate
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations
=======
// a temporary sample buffer that has been constructed with the desired
>>>>>>> Align SampleBuffer with std::vector
// size:
//
//     SampleBuffer sampleBuffer(oldSize);
//     ...
<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> Align SampleBuffer with std::vector
//     SampleBuffer tempBuffer(newSize)
//     ... copy data from sampleBuffer to tempBuffer...
//     sampleBuffer.swap(sampleBuffer);
//
// The local variable tempBuffer can be omitted if no data needs to be
// copied from the existing sampleBuffer:
//
//     SampleBuffer sampleBuffer(oldSize);
//     ...
<<<<<<< HEAD
//     SampleBuffer(newSize).swap(sampleBuffer);
//
class SampleBuffer {
    Q_DISABLE_COPY(SampleBuffer);

public:
    SampleBuffer()
            : m_data(NULL),
              m_size(0) {
    }
    explicit SampleBuffer(SINT size);
    virtual ~SampleBuffer();

    SINT size() const {
        return m_size;
    }

    CSAMPLE* data() {
        return m_data;
    }
    const CSAMPLE* data() const {
        return m_data;
    }

    CSAMPLE& operator[](SINT index) {
        return m_data[index];
    }
    const CSAMPLE& operator[](SINT index) const {
        return m_data[index];
    }

    // Exchanges the members of two buffers in conformance with the
    // implementation of all STL containers. Required for exception
    // safe programming and as a workaround for the missing resize
    // operation.
    void swap(SampleBuffer& other) {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
    }

    // Fills the whole buffer with zeroes
    void clear();

    // Fills the whole buffer with the same value
    void fill(CSAMPLE value);

private:
    CSAMPLE* m_data;
    SINT m_size;
};

namespace std
{

// Template specialization of std::swap for SampleBuffer.
template<>
inline void swap(SampleBuffer& lhs, SampleBuffer& rhs) {
    lhs.swap(rhs);
}

}

#endif // SAMPLEBUFFER_H
=======
=======
>>>>>>> Align SampleBuffer with std::vector
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

<<<<<<< HEAD
#endif /* SAMPLEBUFFER_H */
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations
=======
#endif // SAMPLEBUFFER_H
>>>>>>> Move utility functions for SampleBuffer into separate file
