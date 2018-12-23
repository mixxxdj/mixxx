#pragma once

#include <QAtomicInt>

#include "util/assert.h"

// Lock-free FIFO for multiple producers/writers and a single(!) consumer/reader.
template<typename T, int capacity>
class MpscFifo {
  public:
    MpscFifo()
          : m_enqueueSize(0),
            m_dequeueSize(0),
            m_headIndex(0),
            m_tailIndex(0) {
        static_assert(capacity >= 2, "capacity too low");
    }

    bool enqueue(T value) {
        if (m_enqueueSize.fetchAndAddAcquire(1) >= capacity) {
            // Queue is full -> Restore size and abort
            m_enqueueSize.fetchAndAddRelease(-1);
            return false;
        } else {
            int headIndex = m_headIndex.fetchAndAddAcquire(1);
            DEBUG_ASSERT(headIndex >= 0);
            m_buffer[headIndex % capacity] = std::move(value);
            // Allow the reader to access the enqueued value
            m_dequeueSize.fetchAndAddRelease(1);
            return true;
        }
    }

    bool dequeue(T* value) {
        if (m_dequeueSize.fetchAndAddAcquire(-1) <= 0) {
            // Queue is empty -> Restore size and abort
            m_dequeueSize.fetchAndAddRelease(1);
            return false;
        } else {
            DEBUG_ASSERT(m_tailIndex >= 0);
            DEBUG_ASSERT(m_tailIndex < capacity);
            *value = std::move(m_buffer[m_tailIndex]);
            if (++m_tailIndex >= capacity) {
                m_tailIndex %= capacity;
                // Prevent unlimited growth and overflow of m_headIndex which
                // is already ahead of m_tailIndex.
                int headIndex;
                do {
                    headIndex = m_headIndex.load();
                    DEBUG_ASSERT(headIndex >= 0);
                } while ((headIndex >= capacity) &&
                        !m_headIndex.testAndSetOrdered(headIndex, headIndex % capacity));
            }
            // Allow the writer to overwrite the dequeued value
            m_enqueueSize.fetchAndAddRelease(-1);
            return true;
        }
    }

  private:
    T m_buffer[capacity];
    QAtomicInt m_enqueueSize;
    QAtomicInt m_dequeueSize;
    QAtomicInt m_headIndex;
    int m_tailIndex;
};
