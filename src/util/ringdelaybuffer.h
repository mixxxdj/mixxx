#pragma once

#include "util/samplebuffer.h"
#include "util/types.h"

// TODO(davidchocholaty) Done documentation.

/// The ring delay buffer allows moving with the reading position subject
/// to certain rules. Another difference between the classic ring buffer is,
/// that the ring delay buffer offers to read zero values
/// which were not written by using the write method and write position.
/// Both of these two specific properties are based on the cross-fading
/// between changes of two delays.
class RingDelayBuffer final {
  public:
    RingDelayBuffer(SINT bufferSize);

    virtual ~RingDelayBuffer(){};

    bool isFull() {
        return getWriteAvailable() == 0;
    }

    bool isEmpty() const {
        return m_readPos == m_writePos;
    }

    void clear() {
        m_readPos = 0;
        m_writePos = 0;

        memset(m_buffer.data(), 0, sizeof(m_buffer));
    }

    SINT length() const {
        return m_bufferSize;
    }

    SINT getReadAvailable() {
        // The m_ringFullMask has to be XORed with m_jumpLeftAroundMask
        // to handle the situation, that the read position jump crossed the left side
        // of the delay buffer and the extra bit was set as empty.
        return (m_writePos - m_readPos) & (m_ringFullMask ^ m_jumpLeftAroundMask);
    }

    SINT getWriteAvailable() {
        return m_bufferSize - getReadAvailable();
    }

    SINT read(CSAMPLE* pBuffer, const SINT itemsToRead);
    SINT write(const CSAMPLE* pBuffer, const SINT numItems);
    SINT moveReadPositionBy(SINT jumpSize);

  private:
    // Size of the ring delay buffer.
    SINT m_bufferSize;
    // Position of next readable element.
    SINT m_readPos;
    // Position of next writable element.
    SINT m_writePos;
    // Used for wrapping indices with extra bit to distinguish full/empty.
    SINT m_ringFullMask;
    // Used for fitting indices to buffer.
    SINT m_ringMask;
    // In the case that the read position circled the delay buffer
    // and crossed the left side of the delay buffer, so, for next crossing
    // of the right size of the delay buffer this mask including the full mask
    // is used for masking. It handles specific situations, where the extra bit
    // for write position is empty (so, for the read position too)
    // and the read position jumps to the left over the left delay buffer side.
    SINT m_jumpLeftAroundMask;
    // Ring delay buffer.
    mixxx::SampleBuffer m_buffer;
};
