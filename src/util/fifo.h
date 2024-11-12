#pragma once

#include <cstddef>

#include "util/class.h"
#include "util/math.h"

// Fast, trivial type only single producer single consumer ring buffer, lock and wait free.
// Internal buffer size will be rounded up to a power of two.

// Internally we use std::size_t, but the original API used int for the return values.
// It would be better to use std::size_t also for the return values, but this breaks
// Windows compilation and should be done in a follow-up PR.
using ring_buffer_size_t = int;

template <class DataType>
class FIFO {
  public:
    using size_type = std::size_t;

    explicit FIFO(size_type size)
            : m_size{roundUpToPowerOf2(static_cast<unsigned int>(size))},
              m_mask{m_size - 1},
              m_data(m_size),
              m_writeIndex{0},
              m_readIndex{0} {
    }

    // Returns the number of values in the ringbuffer available for read
    int readAvailable() const {
        const size_type readIndex = m_readIndex.load(std::memory_order_relaxed);
        const size_type writeIndex = m_writeIndex.load(std::memory_order_acquire);
        return static_cast<int>(writeIndex - readIndex);
    }
    // Returns the space in the ringbuffer available for write
    int writeAvailable() const {
        const size_type readIndex = m_readIndex.load(std::memory_order_acquire);
        const size_type writeIndex = m_writeIndex.load(std::memory_order_relaxed);
        return static_cast<int>(m_size - (writeIndex - readIndex));
    }
    // Read count values to the ring buffer. If less then count values are
    // available, only read the available amount. The return value is the
    // actual number of values read. If the read index reached the end of the
    // ringbuffer, the remainder is read from the start.
    int read(DataType* pData, size_type count) {
        size_type readIndex = m_readIndex.load(std::memory_order_relaxed);
        const size_type writeIndex = m_writeIndex.load(std::memory_order_acquire);
        const size_type available = writeIndex - readIndex;
        count = std::min(available, count);
        readIndex = readIndex & m_mask;
        const size_type n = std::min(m_size - readIndex, count);
        std::copy(m_data.data() + readIndex, m_data.data() + readIndex + n, pData);
        std::copy(m_data.data(), m_data.data() + count - n, pData + n);
        m_readIndex.fetch_add(count, std::memory_order_release);
        return static_cast<int>(count);
    }
    // Write count samples to the ring buffer. If space available is less then
    // count, only write the available space amount. The return value is the
    // actual number of values written. If the write index reached the end of the
    // ringbuffer, the remainder is written to the start.
    int write(const DataType* pData, size_type count) {
        const size_type readIndex = m_readIndex.load(std::memory_order_acquire);
        size_type writeIndex = m_writeIndex.load(std::memory_order_relaxed);
        const size_type available = m_size - (writeIndex - readIndex);
        count = std::min(available, count);
        writeIndex = writeIndex & m_mask;
        const size_type n = std::min(m_size - writeIndex, count);
        std::copy(pData, pData + n, m_data.data() + writeIndex);
        std::copy(pData + n, pData + count, m_data.data());
        m_writeIndex.fetch_add(count, std::memory_order_release);
        return static_cast<int>(count);
    }
    void writeBlocking(const DataType* pData, size_type count) {
        size_type written = 0;
        while (written < count) {
            written += write(pData + written, count - written);
        }
    }
    // Give direct access to the ring buffer at the read index
    // to a region of count values. If less than count values
    // are available, limit to the available amount.
    // If the region surpasses the end of the ring buffer,
    // the first region will be until the end, the second
    // region will be the remainder from the start, otherwise
    // the second region will be nullptr and size 0.
    int aquireReadRegions(size_type count,
            DataType** dataPtr1,
            int* sizePtr1,
            DataType** dataPtr2,
            int* sizePtr2) {
        size_type readIndex = m_readIndex.load(std::memory_order_relaxed);
        const size_type writeIndex = m_writeIndex.load(std::memory_order_acquire);
        const size_type available = writeIndex - readIndex;
        count = std::min(available, count);
        readIndex = readIndex & m_mask;
        *sizePtr1 = static_cast<int>(std::min(m_size - readIndex, count));
        *sizePtr2 = static_cast<int>(count) - *sizePtr1;
        *dataPtr1 = m_data.data() + readIndex;
        *dataPtr2 = *sizePtr2 == 0 ? nullptr : m_data.data();

        return static_cast<int>(count);
    }
    // Advance the read index after aquireReadRegions. Count should
    // be the return value of aquireReadRegions, which is the total
    // region size acquired.
    int releaseReadRegions(size_type count) {
        const size_type readIndex = (m_readIndex.load(std::memory_order_relaxed) + count);
        m_readIndex.store(readIndex, std::memory_order_release);
        return static_cast<int>(readIndex & m_mask);
    }
    // Same as aquireReadRegions, for write operations
    int aquireWriteRegions(size_type count,
            DataType** dataPtr1,
            int* sizePtr1,
            DataType** dataPtr2,
            int* sizePtr2) {
        const size_type readIndex = m_readIndex.load(std::memory_order_acquire);
        size_type writeIndex = m_writeIndex.load(std::memory_order_relaxed);
        const size_type available = m_size - (writeIndex - readIndex);
        count = std::min(available, count);
        writeIndex = writeIndex & m_mask;
        *sizePtr1 = static_cast<int>(std::min(m_size - writeIndex, count));
        *sizePtr2 = static_cast<int>(count) - *sizePtr1;
        *dataPtr1 = m_data.data() + writeIndex;
        *dataPtr2 = *sizePtr2 == 0 ? nullptr : m_data.data();

        return static_cast<int>(count);
    }
    // Same as releaseReadRegions, for write operations
    int releaseWriteRegions(size_type count) {
        const size_type writeIndex = (m_writeIndex.load(std::memory_order_relaxed) + count);
        m_writeIndex.store(writeIndex, std::memory_order_release);
        return static_cast<int>(writeIndex & m_mask);
    }
    // Advance the read index with count values, or maximum until the write index.
    // Returns the new read index (wrapped inside the buffer size)
    int flushReadData(size_type count) {
        size_type readIndex = m_readIndex.load(std::memory_order_relaxed);
        const size_type writeIndex = m_writeIndex.load(std::memory_order_acquire);
        const size_type available = writeIndex - readIndex;
        count = std::min(available, count);
        readIndex += count;
        m_readIndex.store(readIndex, std::memory_order_release);
        return static_cast<int>(readIndex & m_mask);
    }

  private:
    const size_type m_size;
    const size_type m_mask;
    std::vector<DataType> m_data;
    std::atomic<size_type> m_writeIndex;
    std::atomic<size_type> m_readIndex;
    // The memory order have the atomic has to be guaranteed:
    // - The read functions have to use memory_order_acquire to access the write
    //   index, while the write functions, which modify the write index, have to
    //   use memory_order_release. This is to ensure that the read function will
    //   not see the write index before it has been modified.
    // - Vice versa the read index has to be accessed with memory_order_acquire
    //   in the write functions and changed with memory_order_release.
    // - All other access can be memory_order_relaxed as no other operations on
    //   the atomics take place in between the operations described above.

    DISALLOW_COPY_AND_ASSIGN(FIFO);
};
