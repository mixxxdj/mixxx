#ifndef SAMPLEBUFFER_H
#define SAMPLEBUFFER_H

#include "util/types.h"

#include <algorithm> // std::swap

// A sample buffer with properly aligned memory to enable SSE optimizations.
// After construction the content of the buffer is uninitialized. No resize
// operation is provided intentionally because malloc might block!
//
// Hint: If the size of an existing sample buffer ever needs to be altered
// after construction this can simply be achieved by swapping the contents
// with a temporary sample buffer that has been constructed with the desired
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
