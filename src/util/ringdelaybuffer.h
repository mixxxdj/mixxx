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
// - size
//
// Modified functions:
// - clear
// - read
// - write

#include "util/samplebuffer.h"
#include "util/types.h"

// The RingDelayBuffer is designed to be single-threaded
// and based on that IS NOT THREAD-SAFE.

/// The class implements the ring buffer with specific constraints,
/// features and optimizations for the "delay handling" use case.
/// The implementation is preferably designed for delay handling
/// of the effect chain which is implemented
/// in "engine/effects/engineeffectsdelay.cpp(.h)".

/// The first main difference between the "classic" ring buffer
/// as widely known and this one is, that the first input items chunk,
/// that is written into the empty ring buffer is ramped (faded-in).
/// This feature is implemented into the ring buffer due to avoid "crackling"
/// when the reader crossed the border between zero silence samples
/// and the real data that was written into the ring buffer. This situation
/// is possible in the mentioned usage of the effect chain delay handling.

/// Based on all of that, the second difference is, that the ring buffer
/// is prefilled with zero "silence" samples in the default state or after
/// clean-up to avoid the reading of uninitialized data.

/// The last main difference is, that the ring buffer stores
/// only the information about the write position, not about the read position.
/// The items are read using the write position, the number of items
/// that should be read and the delay of the reading position
/// against the write position. The reading is optimized this way
/// for the use case of the effect chain delay handling.
class RingDelayBuffer final {
  public:
    RingDelayBuffer(SINT bufferSize);

    /// The method clears the ring delay buffer. That means,
    /// that the ring buffer is filled with zero "silence" samples,
    /// the write position is at the start of the ring buffer (index 0)
    /// and as last, the flag for the first input buffer is set in the state,
    /// that the next chunk of data that will be written, will be faded-in.
    void clear() {
        m_firstInputChunk = true;
        m_writePos = 0;

        m_buffer.fill(0);
    }

    /// The method returns the size of the ring buffer.
    SINT size() const {
        return m_buffer.size();
    }

    /// The method reads items from the ring buffer and stores them
    /// in the pBuffer. The number of items that will be read
    /// is passed through the itemsToRead parameter. To allow delay handling
    /// when reading ring buffer data, the delayItems parameter contains
    /// the delay items that determine the reading position
    /// against the write position. The sum of items to read and the delay items
    /// has to be smaller or equal to the ring buffer size. Otherwise, the items
    /// are not read and the method returns the zero value.
    SINT read(CSAMPLE* pBuffer, const SINT itemsToRead, const SINT delayItems);

    /// The method writes items from the pBuffer into the ring buffer.
    /// The number of items that will be written is passed through
    /// the itemsToWrite parameter. This value has to be smaller or equal
    /// to the ring buffer size. Otherwise, the items are not written
    /// and the method returns the zero value. The first chunk of items
    /// after creating of ring delay buffer or after clean-up is ramped
    /// by using fading-in.
    SINT write(const CSAMPLE* pBuffer, const SINT itemsToWrite);

  private:
    // This flag ensures the "fading in" for the first input chunk
    // into the clear ring delay buffer. It is done to avoid
    // a "crackling" sound when the input samples are read after reading
    // the previous zero samples.
    bool m_firstInputChunk;
    // Position of next writable element.
    SINT m_writePos;
    // Ring delay buffer.
    mixxx::SampleBuffer m_buffer;
};
