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

#include "util/sample.h"

RingDelayBuffer::RingDelayBuffer(SINT bufferSize)
        : m_fullFlag(false),
          m_firstInputBuffer(true),
          m_readPos(0),
          m_writePos(0),
          m_buffer(bufferSize) {
    // Set the ring delay buffer items to 0.
    m_buffer.fill(0);
}

SINT RingDelayBuffer::read(CSAMPLE* pBuffer, const SINT numItems) {
    const SINT available = getReadAvailable();
    SINT itemsToRead = numItems;

    VERIFY_OR_DEBUG_ASSERT(itemsToRead <= available) {
        itemsToRead = available;
    }

    // Check to see if the read is not contiguous.
    if ((m_readPos + itemsToRead) > m_buffer.size()) {
        // Read is not contiguous.
        SINT firstDataBlockSize = m_buffer.size() - m_readPos;

        SampleUtil::copy(pBuffer, m_buffer.data(m_readPos), firstDataBlockSize);
        pBuffer = pBuffer + firstDataBlockSize;

        // The second data part is the start of the ring buffer.
        SampleUtil::copy(pBuffer, m_buffer.data(), itemsToRead - firstDataBlockSize);
    } else {
        // Read is contiguous.
        SampleUtil::copy(pBuffer, m_buffer.data(m_readPos), itemsToRead);
    }

    // Calculate the new read position. If the new read position
    // is after the ring delay buffer end, move it around from the start
    // of the ring delay buffer.
    m_readPos = (m_readPos + itemsToRead);

    if (m_readPos >= m_buffer.size()) {
        m_readPos = m_readPos - m_buffer.size();
        // The read position crossed the right bound of the buffer. So, if the situation,
        // where the read position equals the write position arises, and before this situation,
        // the write position does not cross the right bound of the buffer too,
        // the buffer will be empty.
        m_fullFlag = false;
    }

    return itemsToRead;
}

SINT RingDelayBuffer::write(const CSAMPLE* pBuffer, const SINT numItems) {
    const SINT available = getWriteAvailable();
    SINT itemsToWrite = numItems;

    VERIFY_OR_DEBUG_ASSERT(itemsToWrite <= available) {
        itemsToWrite = available;
    }

    if (m_firstInputBuffer) {
        // If the first input buffer is written, the first sample is on the index 0.
        // Based on the checking of an available number of samples, the situation,
        // that the writing will be non-contiguous cannot occur.
        // The itemsToWrite value is multiply by 2 to
        SampleUtil::copyWithRampingGain(m_buffer.data(), pBuffer, 0.0f, 1.0f, itemsToWrite);

        m_firstInputBuffer = false;
    } else if ((m_writePos + itemsToWrite) > m_buffer.size()) { // Check to see
                                                                // if the write is not contiguous.
        // Write is not contiguous.
        SINT firstDataBlockSize = m_buffer.size() - m_writePos;

        SampleUtil::copy(m_buffer.data(m_writePos), pBuffer, firstDataBlockSize);
        pBuffer = pBuffer + firstDataBlockSize;

        // The second data part is the start of the ring buffer.
        SampleUtil::copy(m_buffer.data(), pBuffer, itemsToWrite - firstDataBlockSize);
    } else {
        // Write is contiguous.
        SampleUtil::copy(m_buffer.data(m_writePos), pBuffer, itemsToWrite);
    }

    // Calculate the new write position. If the new write position
    // is after the ring delay buffer end, move it around from the start
    // of the ring delay buffer.
    m_writePos = (m_writePos + itemsToWrite);

    if (m_writePos >= m_buffer.size()) {
        m_writePos = m_writePos - m_buffer.size();
        // The write position crossed the right bound of the buffer. So, if the situation,
        // where the write position equals the read position arises, and before this situation,
        // the read position does not cross the right bound of the buffer too,
        // the buffer will be full.
        m_fullFlag = true;
    }

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
        // The buffer is empty (all elements have been read).
        m_fullFlag = false;

        return readAvailableRight;
    }

    // Jump to the left cannot be greater than the number of available items
    // (in negative values smaller number).
    VERIFY_OR_DEBUG_ASSERT(jumpSize >= readAvailableLeft) {
        m_readPos = m_writePos;
        // The buffer is full. In this situation, the values on the left side
        // of the read position before the jump are considered as written,
        // even though it is about default zero values. After the jump,
        // the samples have to be read first before samples can be written.
        m_fullFlag = true;

        return readAvailableLeft;
    }

    // Add jump size to current read position.
    m_readPos = m_readPos + jumpSize;

    if (m_readPos >= m_buffer.size()) {
        // Crossing the right bound of the buffer.
        m_readPos = m_readPos - m_buffer.size();

        // The read position crossed the right bound of the buffer. So, if the situation,
        // where the read position equals the write position arises, and before this situation,
        // the write position does not cross the right bound of the buffer too,
        // the buffer will be empty.
        m_fullFlag = false;
    } else if (m_readPos < 0) {
        // Crossing the left bound of the buffer.
        m_readPos = m_readPos + m_buffer.size();

        // The read position crossed the left bound of the buffer. So, if the situation,
        // where the read position equals the write position arises, the buffer will be full.
        // In this situation, the values on the left side of the read position before the jump
        // are considered as written, even though it is about default zero values.
        m_fullFlag = true;
    }

    return jumpSize;
}
