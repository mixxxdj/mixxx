#include "fifosamplebuffer.h"

#include "sampleutil.h"

#define DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer() \
    DEBUG_ASSERT(0 <= m_readOffset); \
    DEBUG_ASSERT(m_readOffset <= m_writeOffset); \
    DEBUG_ASSERT(m_writeOffset <= m_sampleBuffer.size()); \
    DEBUG_ASSERT(m_sampleBuffer.size() == m_shadowBuffer.size())

FifoSampleBuffer::FifoSampleBuffer()
        : m_readOffset(0),
          m_writeOffset(0) {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
}

FifoSampleBuffer::FifoSampleBuffer(SINT capacity)
    : m_sampleBuffer(capacity),
      m_shadowBuffer(capacity),
      m_readOffset(0),
      m_writeOffset(0) {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
}

void FifoSampleBuffer::swap(FifoSampleBuffer& other) {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    std::swap(m_sampleBuffer, other.m_sampleBuffer);
    std::swap(m_shadowBuffer, other.m_shadowBuffer);
    std::swap(m_readOffset, other.m_readOffset);
    std::swap(m_writeOffset, other.m_writeOffset);
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
}

void FifoSampleBuffer::swapBuffers() {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    SampleUtil::copy(
            m_shadowBuffer.data(),
            m_sampleBuffer.data() + m_readOffset,
            m_writeOffset - m_readOffset);
    m_sampleBuffer.swap(m_shadowBuffer);
    m_writeOffset -= m_readOffset;
    m_readOffset = 0;
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
}

void FifoSampleBuffer::reset() {
    m_readOffset = 0;
    m_writeOffset = 0;
}

void FifoSampleBuffer::trim() {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    if (0 < m_readOffset) {
        if (isEmpty()) {
            reset();
        } else {
            swapBuffers();
        }
    }
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
}

std::pair<CSAMPLE*, SINT> FifoSampleBuffer::growTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    if (isEmpty()) {
        reset();
    } else {
        if ((m_sampleBuffer.size() - m_writeOffset) < size) {
            trim();
        }
    }
    CSAMPLE* const resultData = m_sampleBuffer.data() + m_writeOffset;
    const SINT resultSize = math_min(size, m_sampleBuffer.size() - m_writeOffset);
    m_writeOffset += resultSize;
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    return std::make_pair(resultData, resultSize);
}

SINT FifoSampleBuffer::shrinkTail(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    const SINT resultSize = math_min(size, getSize());
    m_writeOffset -= resultSize;
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    return resultSize;
}

std::pair<const CSAMPLE*, SINT> FifoSampleBuffer::shrinkHead(SINT size) {
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    const CSAMPLE* const resultData = m_sampleBuffer.data() + m_readOffset;
    const SINT resultSize = math_min(size, getSize());
    m_readOffset += resultSize;
    DEBUG_ASSERT_CLASS_INVARIANT_FifoSampleBuffer();
    return std::make_pair(resultData, resultSize);
}
