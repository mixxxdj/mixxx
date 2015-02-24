#ifndef SAMPLEBUFFERUTIL_H
#define SAMPLEBUFFERUTIL_H

#include "samplebuffer.h"
#include "sampleutil.h"

// Fills the whole buffer with zeroes
inline void clear(SampleBuffer* pBuffer) {
    SampleUtil::clear(pBuffer->data(), pBuffer->size());
}

// Fills the whole buffer with the same value
inline void fill(SampleBuffer* pBuffer, SampleBuffer::value_type value) {
    SampleUtil::fill(pBuffer->data(), value, pBuffer->size());
}

#endif // SAMPLEBUFFERUTIL_H
