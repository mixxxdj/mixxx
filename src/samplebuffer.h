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
    typedef CSAMPLE value_type;
    typedef int size_type;

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
    const value_type& operator[](size_type index) const {
        return m_data[index];
    }

  private:
    value_type* m_data;
    size_type m_size;
};

#endif // SAMPLEBUFFER_H
