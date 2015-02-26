#include "samplebuffer.h"

#include "sampleutil.h"

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
SampleBuffer::SampleBuffer(SINT size)
=======

SampleBuffer::SampleBuffer(size_type size)
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations
=======
SampleBuffer::SampleBuffer(int size)
>>>>>>> Update SampleBuffer
=======
SampleBuffer::SampleBuffer(SINT size)
>>>>>>> Update SampleBuffer: Use SINT
        : m_data(SampleUtil::alloc(size)),
          m_size(m_data ? size : 0) {
}

SampleBuffer::~SampleBuffer() {
    SampleUtil::free(m_data);
}
<<<<<<< HEAD
<<<<<<< HEAD

void SampleBuffer::clear() {
<<<<<<< HEAD
=======

void SampleBuffer::clear() {
>>>>>>> Update SampleBuffer
    SampleUtil::clear(data(), size());
}

// Fills the whole buffer with the same value
void SampleBuffer::fill(CSAMPLE value) {
    SampleUtil::fill(data(), value, size());
<<<<<<< HEAD
=======
    SampleUtil::clear(m_data, m_size);
}

void SampleBuffer::fill(value_type value) {
    SampleUtil::fill(m_data, value, m_size);
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations
}
=======
>>>>>>> Move utility functions for SampleBuffer into separate file
=======
}
>>>>>>> Update SampleBuffer
