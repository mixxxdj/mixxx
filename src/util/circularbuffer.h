#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <stdlib.h>

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
            : m_iLength(iLength),
              m_pBuffer(new T[m_iLength]),
              m_iWritePos(0),
              m_iReadPos(0) {
        // No need to clear the buffer because we consider it to be empty right
        // now.
    }

    virtual ~CircularBuffer() {
        delete [] m_pBuffer;
        m_pBuffer = NULL;
    }

    // Returns true if the buffer is full
    inline bool isFull() const {
        return (m_iWritePos + 1) % m_iLength == m_iReadPos;
    }

    // Returns true if the buffer is empty.
    inline bool isEmpty() const {
        return m_iReadPos == m_iWritePos;
    }

    inline void clear() {
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
        if (m_pBuffer == NULL)
            return 0;

        unsigned int itemsWritten = 0;
        while (!isFull() && itemsWritten < numItems) {
            m_pBuffer[m_iWritePos++] = pBuffer[itemsWritten++];
            m_iWritePos %= m_iLength;
        }
        return itemsWritten;
    }

    // Read itemsToRead into pBuffer. Returns the total number of items read,
    // which may be less than itemsToRead if the buffer becomes empty.
    unsigned int read(T* pBuffer, const unsigned int itemsToRead) {
        if (m_pBuffer == NULL)
            return 0;

        unsigned int itemsRead = 0;
        while (!isEmpty() && itemsRead < itemsToRead) {
            pBuffer[itemsRead++] = m_pBuffer[m_iReadPos++];
            m_iReadPos %= m_iLength;
        }
        return itemsRead;
    }

    unsigned int skip(const unsigned int itemsToRead) {
        if (m_pBuffer == NULL)
            return 0;
        unsigned int itemsRead = 0;
        while (!isEmpty() && itemsRead < itemsToRead) {
            m_iReadPos = (m_iReadPos + 1) % m_iLength;
            itemsRead++;
        }
        return itemsRead;
    }

  private:
    const unsigned int m_iLength;
    T* m_pBuffer;
    unsigned int m_iWritePos;
    unsigned int m_iReadPos;
};

#endif /* CIRCULARBUFFER_H */
