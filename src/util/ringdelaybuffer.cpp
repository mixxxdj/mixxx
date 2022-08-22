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

#include "util/math.h"
#include "util/sample.h"

using ReadableSlice = mixxx::SampleBuffer::ReadableSlice;
using WritableSlice = mixxx::SampleBuffer::WritableSlice;

namespace {
void copyRing(const ReadableSlice pSourceBuffer,
        SINT sourcePos,
        const WritableSlice pDestBuffer,
        SINT destPos,
        const SINT numItems) {
    // Check to see if the copy is not contiguous.
    if (sourcePos + numItems > pSourceBuffer.length() ||
            destPos + numItems > pDestBuffer.length()) {
        // Copy is not contiguous.
        SINT firstDataBlockSize = math_min(pSourceBuffer.length() - sourcePos,
                pDestBuffer.length() - destPos);

        SampleUtil::copy(pDestBuffer.data(destPos),
                pSourceBuffer.data(sourcePos),
                firstDataBlockSize);

        sourcePos = (sourcePos + firstDataBlockSize) % pSourceBuffer.length();
        destPos = (destPos + firstDataBlockSize) % pDestBuffer.length();

        // The second data part is the start of the ring buffer.
        SampleUtil::copy(pDestBuffer.data(destPos),
                pSourceBuffer.data(sourcePos),
                numItems - firstDataBlockSize);
    } else {
        // Copy is contiguous.
        SampleUtil::copy(pDestBuffer.data(destPos),
                pSourceBuffer.data(sourcePos),
                numItems);
    }
}

} // anonymous namespace

RingDelayBuffer::RingDelayBuffer(SINT bufferSize)
        : m_firstInputBuffer(true),
          m_writePos(0),
          m_buffer(bufferSize) {
    // Set the ring delay buffer items to 0.
    m_buffer.fill(0);
}

SINT RingDelayBuffer::read(CSAMPLE* pBuffer, const SINT itemsToRead, const SINT delayItems) {
    const SINT shift = itemsToRead + delayItems;

    VERIFY_OR_DEBUG_ASSERT(shift <= m_buffer.size()) {
        return 0;
    }

    SINT readPos = m_writePos - shift;

    if (readPos < 0) {
        readPos = readPos + m_buffer.size();
    }

    copyRing(ReadableSlice(m_buffer.data(), m_buffer.size()),
            readPos,
            WritableSlice(pBuffer, itemsToRead),
            0,
            itemsToRead);

    return itemsToRead;
}

SINT RingDelayBuffer::write(const CSAMPLE* pBuffer, const SINT itemsToWrite) {
    VERIFY_OR_DEBUG_ASSERT(itemsToWrite <= m_buffer.size()) {
        return 0;
    }

    if (m_firstInputBuffer) {
        // If the first input buffer is written, the first sample is on the index 0.
        // Based on the checking of an available number of samples, the situation,
        // that the writing will be non-contiguous cannot occur.
        // The itemsToWrite value is multiply by 2 to
        SampleUtil::copyWithRampingGain(m_buffer.data(), pBuffer, 0.0f, 1.0f, itemsToWrite);
        m_firstInputBuffer = false;
    } else {
        copyRing(ReadableSlice(pBuffer, itemsToWrite),
                0,
                WritableSlice(m_buffer.data(), m_buffer.size()),
                m_writePos,
                itemsToWrite);
    }

    // Calculate the new write position. If the new write position
    // is after the ring delay buffer end, move it around from the start
    // of the ring delay buffer.
    m_writePos = (m_writePos + itemsToWrite);

    if (m_writePos >= m_buffer.size()) {
        m_writePos = m_writePos - m_buffer.size();
    }

    return itemsToWrite;
}
