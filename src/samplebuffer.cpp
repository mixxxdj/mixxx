#include "samplebuffer.h"

#include "sampleutil.h"

<<<<<<< HEAD
SampleBuffer::SampleBuffer(SINT size)
=======

SampleBuffer::SampleBuffer(size_type size)
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations
        : m_data(SampleUtil::alloc(size)),
          m_size(m_data ? size : 0) {
}

SampleBuffer::~SampleBuffer() {
    SampleUtil::free(m_data);
}
<<<<<<< HEAD

void SampleBuffer::clear() {
<<<<<<< HEAD
    SampleUtil::clear(data(), size());
}

// Fills the whole buffer with the same value
void SampleBuffer::fill(CSAMPLE value) {
    SampleUtil::fill(data(), value, size());
=======
    SampleUtil::clear(m_data, m_size);
}

void SampleBuffer::fill(value_type value) {
    SampleUtil::fill(m_data, value, m_size);
>>>>>>> Memory-aligned SampleBuffer to utilize SSE optimizations
}
=======
>>>>>>> Move utility functions for SampleBuffer into separate file
