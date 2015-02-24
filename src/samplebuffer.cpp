#include "samplebuffer.h"

#include "sampleutil.h"


SampleBuffer::SampleBuffer(size_type size)
        : m_data(SampleUtil::alloc(size)),
          m_size(m_data ? size : 0) {
}

SampleBuffer::~SampleBuffer() {
    SampleUtil::free(m_data);
}
