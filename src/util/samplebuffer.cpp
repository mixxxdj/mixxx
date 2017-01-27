#include "util/samplebuffer.h"

#include "util/sample.h"

SampleBuffer::SampleBuffer(SINT size)
        : m_data(SampleUtil::alloc(size)),
          m_size((nullptr != m_data) ? size : 0) {
}

SampleBuffer::~SampleBuffer() {
    SampleUtil::free(m_data);
}

void SampleBuffer::clear() {
    SampleUtil::clear(data(), size());
}

// Fills the whole buffer with the same value
void SampleBuffer::fill(CSAMPLE value) {
    SampleUtil::fill(data(), value, size());
}
