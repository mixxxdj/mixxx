#ifndef MIXXX_UTIL_SAMPLEBUFFER_H
#define MIXXX_UTIL_SAMPLEBUFFER_H

#include <algorithm> // std::swap

#include "util/types.h"

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
  public:
    SampleBuffer()
            : m_data(nullptr),
              m_size(0) {
    }
    explicit SampleBuffer(SINT size);
    SampleBuffer(SampleBuffer& other) = delete;
    SampleBuffer(SampleBuffer&& other)
        : m_data(other.m_data),
          m_size(other.m_size) {
        other.m_data = nullptr;
        other.m_size = 0;
    }
    virtual ~SampleBuffer();

    SampleBuffer& operator=(SampleBuffer&& other) {
        swap(other);
        return *this;
    }

    SINT size() const {
        return m_size;
    }

    CSAMPLE* data(SINT offset = 0) {
        DEBUG_ASSERT(0 <= offset);
        // >=: allow access to one element behind allocated memory
        DEBUG_ASSERT(m_size >= offset);
        return m_data + offset;
    }
    const CSAMPLE* data(SINT offset = 0) const {
        DEBUG_ASSERT(0 <= offset);
        // >=: allow access to one element behind allocated memory
        DEBUG_ASSERT(m_size >= offset);
        return m_data + offset;
    }

    CSAMPLE& operator[](SINT index) {
        return *data(index);
    }
    const CSAMPLE& operator[](SINT index) const {
        return *data(index);
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

    class ReadableChunk {
      public:
        ReadableChunk(const SampleBuffer& buffer, SINT offset, SINT length)
            : m_data(buffer.data(offset)),
              m_size(length) {
            DEBUG_ASSERT((buffer.size() - offset) >= length);
        }
        const CSAMPLE* data(SINT offset = 0) const {
            DEBUG_ASSERT(0 <= offset);
            // >=: allow access to one element behind allocated memory
            DEBUG_ASSERT(m_size >= offset);
            return m_data + offset;
        }
        SINT size() const {
            return m_size;
        }
        const CSAMPLE& operator[](SINT index) const {
            return *data(index);
        }
      private:
        const CSAMPLE* m_data;
        SINT m_size;
    };

    class WritableChunk {
      public:
        WritableChunk(SampleBuffer& buffer, SINT offset, SINT length)
            : m_data(buffer.data(offset)),
              m_size(length) {
            DEBUG_ASSERT((buffer.size() - offset) >= length);
        }
        CSAMPLE* data(SINT offset = 0) const {
            DEBUG_ASSERT(0 <= offset);
            // >=: allow access to one element behind allocated memory
            DEBUG_ASSERT(m_size >= offset);
            return m_data + offset;
        }
        SINT size() const {
            return m_size;
        }
        CSAMPLE& operator[](SINT index) const {
            return *data(index);
        }
      private:
        CSAMPLE* m_data;
        SINT m_size;
    };

  private:
    CSAMPLE* m_data;
    SINT m_size;
};

namespace std {

// Template specialization of std::swap for SampleBuffer.
template<>
inline void swap(SampleBuffer& lhs, SampleBuffer& rhs) {
    lhs.swap(rhs);
}

}  // namespace std

#endif // MIXXX_UTIL_SAMPLEBUFFER_H
