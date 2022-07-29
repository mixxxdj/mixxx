/*
* $Id$
* Portable Audio I/O Library
* Ring Buffer utility.
*
* Author: Phil Burk, http://www.softsynth.com
* modified for SMP safety on OS X by Bjorn Roche.
* also allowed for const where possible.
* modified for multiple-byte-sized data elements by Sven Fischer
*
* This program is distributed with the PortAudio Portable Audio Library.
* For more information see: http://www.portaudio.com
* Copyright (c) 1999-2000 Ross Bencina and Phil Burk
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
* ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
* CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
* The text above constitutes the entire PortAudio license; however,
* the PortAudio community also makes the following non-binding requests:
*
* Any person wishing to distribute modifications to the Software is
* requested to send the modifications to the original developer so that
* they can be incorporated into the canonical version. It is also
* requested that these non-binding requests be included along with the
* license above.
*/

#include "ringdelaybuffer.h"

RingDelayBuffer::RingDelayBuffer(SINT bufferSize)
        : m_readPos(0),
          m_writePos(0),
          m_ringFullMask((bufferSize * 2) - 1),
          m_ringMask(bufferSize - 1),
          m_jumpLeftAroundMask(0),
          m_buffer(bufferSize) {
    // TODO(davidchocholaty) Handle to allow only power of two for the size of the ring delay buffer.

    // Set the ring delay buffer items to 0.
    m_buffer.fill(0);
}

SINT RingDelayBuffer::read(CSAMPLE* pBuffer, const SINT numItems) {
    const SINT available = getReadAvailable();
    SINT itemsToRead = numItems;

    VERIFY_OR_DEBUG_ASSERT(itemsToRead <= available) {
        itemsToRead = available;
    }

    const SINT position = m_readPos & m_ringMask;

    // Check to see if the read is not contiguous.
    if ((position + itemsToRead) > m_buffer.size()) {
        // Read is not contiguous.
        SINT firstDataBlockSize = m_buffer.size() - position;

        memcpy(pBuffer, m_buffer.data(position), firstDataBlockSize * sizeof(CSAMPLE));
        pBuffer = pBuffer + firstDataBlockSize;

        // The second data part is the start of the ring buffer.
        memcpy(pBuffer, m_buffer.data(), (itemsToRead - firstDataBlockSize) * sizeof(CSAMPLE));
    } else {
        // Read is contiguous.
        memcpy(pBuffer, m_buffer.data(position), itemsToRead * sizeof(CSAMPLE));
    }

    // Calculate the new read position. If the new read position
    // is after the ring delay buffer end, move it around from the start
    // of the ring delay buffer. If the extra bit for the read position
    // is set as empty and the jump will cross the right side of the buffer,
    // the m_ringFullMask has to be XORed with the m_jumpLeftAroundMask,
    // to handle the situation, when the read position extra bit can't be set
    // as full for jumping to left. Otherwise, use only the m_ringFullMask
    // for masking.
    if ((position > m_buffer.size()) && (m_readPos < m_buffer.size())) {
        // position > m_buffer.size()  -> cross the right side of the buffer
        // m_readPos < m_buffer.size() -> extra bit is set as empty
        m_readPos = (m_readPos + itemsToRead) & (m_ringFullMask ^ m_jumpLeftAroundMask);
        m_jumpLeftAroundMask = 0;
    } else {
        m_readPos = (m_readPos + itemsToRead) & m_ringFullMask;
    }

    return itemsToRead;
}

SINT RingDelayBuffer::write(const CSAMPLE* pBuffer, const SINT numItems) {
    const SINT available = getWriteAvailable();
    SINT itemsToWrite = numItems;

    VERIFY_OR_DEBUG_ASSERT(numItems <= available) {
        itemsToWrite = available;
    }

    const SINT position = m_writePos & m_ringMask;

    // Check to see if the write is not contiguous.
    if ((position + itemsToWrite) > m_buffer.size()) {
        // Write is not contiguous.
        SINT firstDataBlockSize = m_buffer.size() - position;

        memcpy(m_buffer.data(position), pBuffer, firstDataBlockSize * sizeof(CSAMPLE));
        pBuffer = pBuffer + firstDataBlockSize;

        // The second data part is the start of the ring buffer.
        memcpy(m_buffer.data(), pBuffer, (itemsToWrite - firstDataBlockSize) * sizeof(CSAMPLE));
    } else {
        // Write is contiguous.
        memcpy(m_buffer.data(position), pBuffer, itemsToWrite * sizeof(CSAMPLE));
    }

    // Calculate the new write position. If the new write position
    // is after the ring delay buffer end, move it around from the start
    // of the ring delay buffer.
    m_writePos = (m_writePos + itemsToWrite) & m_ringFullMask;

    return itemsToWrite;
}

SINT RingDelayBuffer::moveReadPositionBy(const SINT jumpSize) {
    if (jumpSize == 0) {
        return 0;
    }

    // For the positive jumpSize values, the jump way is to right.
    // For the negative jumpSize values, the jump way is to left.

    // The number of available elements on the right side
    // of the current read position.
    const SINT readAvailableRight = getReadAvailable();

    // The number of available elements on the left side
    // of the current read position.
    const SINT readAvailableLeft = -getWriteAvailable() + 1;

    // Jump to the right is cannot be greater than the number
    // of available items.
    VERIFY_OR_DEBUG_ASSERT(jumpSize <= readAvailableRight) {
        m_readPos = m_writePos;
        m_jumpLeftAroundMask = 0;

        return readAvailableRight;
    }

    // Jump to the left cannot be greater than the number of available items
    // (in negative values smaller number).
    VERIFY_OR_DEBUG_ASSERT(jumpSize >= readAvailableLeft) {
        m_readPos = m_writePos;
        m_jumpLeftAroundMask = 0;

        return readAvailableLeft;
    }

    if (m_readPos + jumpSize < 0) {
        // The case if the read position full/empty bit is empty
        // and the jump is to left and crosses the left side
        // of the delay buffer around. This case is special.
        // Then, for the crossing of the right side of the delay buffer,
        // the full/empty extra bit can't be set on full as normal.
        // For handling this situation, the m_jumpLeftAroundMask variable
        // is created. If the write position extra bit is empty,
        // this mask contains only the extra full bit. Otherwise,
        // if the write position extra bit is full, the variable value is zero.
        // For the case, that the jump crossed the left side
        // of the delay buffer for the mentioned case, so, when then
        // the read position crosses the right side of the delay buffer,
        // the m_ringFullMask is XORed with the m_jumpLeftAroundMask.
        // This computation step solves the problem of not setting
        // the extra bit on full for the read position.
        m_readPos = (m_readPos + jumpSize) & m_ringMask;
        m_jumpLeftAroundMask = m_buffer.size() & (~m_writePos);
    } else {
        // This branch contains all other cases that can be process normally.
        // If the extra bit for the read position is set as empty
        // and the jump will cross the right side of the buffer,
        // the m_ringFullMask has to be XORed with the m_jumpLeftAroundMask,
        // to handle the situation, when the read position extra bit can't be set
        // as full for jumping to left. Otherwise, use only the m_ringFullMask
        // for masking.
        const SINT position = m_readPos & m_ringMask;

        if ((position > m_buffer.size()) && (m_readPos < m_buffer.size())) {
            // position > m_buffer.size()  -> cross the right side of the buffer
            // m_readPos < m_buffer.size() -> extra bit is set as empty
            m_readPos = (m_readPos + jumpSize) & (m_ringFullMask ^ m_jumpLeftAroundMask);
            m_jumpLeftAroundMask = 0;
        } else {
            m_readPos = (m_readPos + jumpSize) & m_ringFullMask;
        }
    }

    return jumpSize;
}
