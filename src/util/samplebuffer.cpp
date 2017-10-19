#include "util/samplebuffer.h"

#include "util/sample.h"


namespace mixxx {

SampleBuffer::SampleBuffer(SINT size)
        : m_data((size > 0) ? SampleUtil::alloc(size) : nullptr),
          m_size((m_data != nullptr) ? size : 0) {
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

} // namespace mixxx
