#pragma once

#include <cstdlib>
#include <vector>

// CircularBuffer is a basic implementation of a constant-length circular
// buffer.
//
// WARNING: CircularBuffer IS NOT THREAD SAFE! It is "sort of" thread safe on
// platforms with atomic writes and aligned memory locations, but it is most
// definitely not safe on x86 without memory barriers that could re-order reads
// and writes.
template <typename T>
class CircularBuffer {
  public:
    CircularBuffer(unsigned int iLength)
            : m_fullFlag(false),
              m_iLength(iLength),
              m_pBuffer(m_iLength),
              m_iWritePos(0),
              m_iReadPos(0) {
        // No need to clear the buffer because we consider it to be empty right
        // now.
    }

    virtual ~CircularBuffer() = default;

    // Returns true if the buffer is full
    inline bool isFull() const {
        return m_fullFlag;
    }

    // Returns true if the buffer is empty.
    inline bool isEmpty() const {
        return getReadSpace() == 0;
    }

    unsigned int getReadSpace() const {
        if (m_iWritePos > m_iReadPos) {
            return m_iWritePos - m_iReadPos;
        } else if (m_iWritePos < m_iReadPos) {
            return (m_iLength - m_iReadPos) + m_iWritePos;
        } else {
            // The write position equals the read position (m_writePos == m_readPos).
            // The buffer is full or empty (m_fullFlag).
            if (m_fullFlag) {
                return m_iLength;
            }

            return 0;
        }
    }

    unsigned int getWriteSpace() const {
        return m_iLength - getReadSpace();
    }

    inline void clear() {
        m_fullFlag = false;
        m_iReadPos = 0;
        m_iWritePos = 0;
    }

    // Returns the total capacity of the CircularBuffer in units of T
    inline unsigned int length() const {
        return m_iLength;
    }

    // Write numItems into the CircularBuffer. Returns the total number of
    // items written, which could be less than numItems if the buffer becomes
    // full.
    unsigned int write(const T* pBuffer, const unsigned int numItems) {
        if (isFull()) {
            return 0;
        }

        const unsigned int available = getWriteSpace();

        if (numItems > available) {
            return 0;
        }

        if (numItems == available) {
            m_fullFlag = true;
        }

        for (unsigned int itemsWritten = 0; itemsWritten < numItems; ++itemsWritten) {
            m_pBuffer[m_iWritePos] = pBuffer[itemsWritten];
            m_iWritePos = (m_iWritePos + 1) % m_iLength;
        }

        return numItems;
    }

    // Read itemsToRead into pBuffer. Returns the total number of items read,
    // which may be less than itemsToRead if the buffer becomes empty.
    unsigned int read(T* pBuffer, const unsigned int itemsToRead) {
        if (isEmpty()) {
            return 0;
        }

        const unsigned int available = getReadSpace();

        if (itemsToRead > available) {
            return 0;
        }

        if (m_fullFlag && itemsToRead > 0) {
            m_fullFlag = false;
        }

        for (unsigned int itemsRead = 0; itemsRead < itemsToRead; ++itemsRead) {
            pBuffer[itemsRead] = m_pBuffer[m_iReadPos];
            m_iReadPos = (m_iReadPos + 1) % m_iLength;
        }

        return itemsToRead;
    }

    unsigned int skip(const unsigned int itemsToRead) {
        if (isEmpty()) {
            return 0;
        }

        const unsigned int available = getReadSpace();

        if (itemsToRead > available) {
            return 0;
        }

        if (m_fullFlag && itemsToRead > 0) {
            m_fullFlag = false;
        }

        for (unsigned int itemsRead = 0; itemsRead < itemsToRead; ++itemsRead) {
            m_iReadPos = (m_iReadPos + 1) % m_iLength;
        }

        return itemsToRead;
    }

  private:
    // The two special cases can occur if the read position
    // equals the write position: the buffer is full or empty.
    // The full flag serves to distinguish between the mentioned two cases.
    bool m_fullFlag;
    const unsigned int m_iLength;
    std::vector<T> m_pBuffer;
    unsigned int m_iWritePos;
    unsigned int m_iReadPos;
};
