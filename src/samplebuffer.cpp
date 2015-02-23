#include "samplebuffer.h"

#include "sampleutil.h"


SampleBuffer::SampleBuffer(size_type size)
        : m_data(SampleUtil::alloc(size)),
          m_size(m_data ? size : 0) {
}

SampleBuffer::~SampleBuffer() {
    SampleUtil::free(m_data);
}

void SampleBuffer::clear() {
    SampleUtil::clear(m_data, m_size);
}

void SampleBuffer::fill(value_type value) {
    SampleUtil::fill(m_data, value, m_size);
}
