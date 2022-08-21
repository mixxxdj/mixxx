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

#pragma once

// The RingDelayBuffer code is based on the pa_ringbuffer.c
// from the Portable Audio I/O Library. The template code is used
// in the Mixxx.
//
// For the pa_ringbuffer.c see: mixxx/lib/portaudio/pa_ringbuffer.c
//
// Added functions:
// - isFull
// - isEmpty
// - length
// - getReadAvailable
// - getWriteAvailable
// - moveReadPositionBy
//
// Modified functions:
// - clear
// - read
// - write

#include "util/samplebuffer.h"
#include "util/types.h"

// The RingDelayBuffer is designed to be single-threaded
// and based on that IS NOT THREAD-SAFE.

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

    void clear() {
        m_firstInputBuffer = true;
        m_writePos = 0;

        m_buffer.fill(0);
    }

    SINT size() const {
        return m_buffer.size();
    }

    SINT read(CSAMPLE* pBuffer, const SINT itemsToRead, const SINT delayItems);
    SINT write(const CSAMPLE* pBuffer, const SINT itemsToWrite);

  private:
    void copy(const mixxx::SampleBuffer::ReadableSlice pSourceBuffer,
            SINT sourcePos,
            const mixxx::SampleBuffer::WritableSlice pDestBuffer,
            SINT destPos,
            const SINT numItems);

    // This flag ensures the "fading in" for the first input buffer
    // into the clear ring delay buffer. It is done to avoid
    // a crackling sound when the input samples are read after reading
    // the previous zero samples.
    bool m_firstInputBuffer;
    // Position of next writable element.
    SINT m_writePos;
    // Ring delay buffer.
    mixxx::SampleBuffer m_buffer;
};
