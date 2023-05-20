#pragma once


#include <algorithm> // std::swap

#include "util/span.h"
#include "util/types.h"


namespace mixxx {

// A sample buffer with properly aligned memory to enable SSE optimizations.
// After construction the content of the buffer is uninitialized. No resize
// operation is provided intentionally because malloc might block! Copying
// has intentionally been disabled, because it should not be needed.
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
class SampleBuffer final {
  public:
    SampleBuffer()
        : m_data(nullptr),
          m_size(0) {
    }
    explicit SampleBuffer(SINT size);
    SampleBuffer(SampleBuffer&) = delete;
    SampleBuffer(SampleBuffer&& that)
        : m_data(that.m_data),
          m_size(that.m_size) {
        that.m_data = nullptr;
        that.m_size = 0;
    }
    virtual ~SampleBuffer() final;

    SampleBuffer& operator=(SampleBuffer& that) = delete;
    SampleBuffer& operator=(SampleBuffer&& that) {
        swap(that);
        return *this;
    }

    constexpr SINT size() const {
        return m_size;
    }

    CSAMPLE* data(SINT offset = 0) {
        DEBUG_ASSERT((m_data != nullptr) || (offset == 0));
        DEBUG_ASSERT(0 <= offset);
        // >=: allow access to one element behind allocated memory
        DEBUG_ASSERT(m_size >= offset);
        return m_data + offset;
    }
    const CSAMPLE* data(SINT offset = 0) const {
        DEBUG_ASSERT((m_data != nullptr) || (offset == 0));
        DEBUG_ASSERT(0 <= offset);
        // >=: allow access to one element behind allocated memory
        DEBUG_ASSERT(m_size >= offset);
        return m_data + offset;
    }

    std::span<CSAMPLE> span() {
        return mixxx::spanutil::spanFromPtrLen(m_data, m_size);
    }
    std::span<const CSAMPLE> span() const {
        return mixxx::spanutil::spanFromPtrLen(m_data, m_size);
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
    void swap(SampleBuffer& that) {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
    }

    // Fills the whole buffer with zeroes
    void clear();

    // Fills the whole buffer with the same value
    void fill(CSAMPLE value);

    class ReadableSlice {
      public:
        ReadableSlice()
            : m_data(nullptr),
              m_length(0) {
        }
        ReadableSlice(const CSAMPLE* data, SINT length)
            : m_data(data),
              m_length(length) {
            DEBUG_ASSERT(m_length >= 0);
            DEBUG_ASSERT((m_length == 0) || (m_data != nullptr));
        }
        ReadableSlice(const SampleBuffer& buffer, SINT offset, SINT length)
            : m_data(buffer.data(offset)),
              m_length(length) {
            DEBUG_ASSERT((buffer.size() - offset) >= length);
        }
        const CSAMPLE* data(SINT offset = 0) const {
            DEBUG_ASSERT((m_data != nullptr) || (offset == 0));
            DEBUG_ASSERT(0 <= offset);
            // >=: allow access to one element behind allocated memory
            DEBUG_ASSERT(m_length >= offset);
            return m_data + offset;
        }
        SINT length(SINT offset = 0) const {
            DEBUG_ASSERT(0 <= offset);
            // >=: allow access to one element behind allocated memory
            DEBUG_ASSERT(m_length >= offset);
            return m_length - offset;
        }
        bool empty() const {
            return (m_data == nullptr) || (m_length <= 0);
        }
        const CSAMPLE& operator[](SINT index) const {
            return *data(index);
        }
      private:
        const CSAMPLE* m_data;
        SINT m_length;
    };

    class WritableSlice {
      public:
        WritableSlice()
            : m_data(nullptr),
              m_length(0) {
        }
        WritableSlice(CSAMPLE* data, SINT length)
            : m_data(data),
              m_length(length) {
            DEBUG_ASSERT(m_length >= 0);
            DEBUG_ASSERT((m_length == 0) || (m_data != nullptr));
        }
        explicit WritableSlice(SampleBuffer& buffer)
            : m_data(buffer.data()),
              m_length(buffer.size()) {
        }
        WritableSlice(SampleBuffer& buffer, SINT offset, SINT length)
            : m_data(buffer.data(offset)),
              m_length(length) {
            DEBUG_ASSERT((buffer.size() - offset) >= length);
        }
        CSAMPLE* data(SINT offset = 0) const {
            DEBUG_ASSERT((m_data != nullptr) || (offset == 0));
            DEBUG_ASSERT(0 <= offset);
            // >=: allow access to one element behind allocated memory
            DEBUG_ASSERT(m_length >= offset);
            return m_data + offset;
        }
        SINT length(SINT offset = 0) const {
            DEBUG_ASSERT(0 <= offset);
            // >=: allow access to one element behind allocated memory
            DEBUG_ASSERT(m_length >= offset);
            return m_length - offset;
        }
        bool empty() const {
            return (m_data == nullptr) || (m_length <= 0);
        }
        CSAMPLE& operator[](SINT index) const {
            return *data(index);
        }
      private:
        CSAMPLE* m_data;
        SINT m_length;
    };

  private:
    CSAMPLE* m_data;
    SINT m_size;
};

} // namespace mixxx

namespace std {

// Template specialization of std::swap() for SampleBuffer
template<>
inline void swap(::mixxx::SampleBuffer& lhs, ::mixxx::SampleBuffer& rhs) {
    lhs.swap(rhs);
}

}  // namespace std
