#pragma once

#include <QAtomicInt>
#include <QMutexLocker>

#include "util/assert.h"
#include "util/memory.h"

enum class MpscFifoConcurrency {
    SingleProducer,
    MultipleProducers,
};

// FIFO for multiple producers/writers and a single consumer/reader. Reading
// is lock-free while concurrent writers are synchronized by a mutex. The
// mutex can be disabled by explicitly creating a single producer instance.
template<typename T, int capacity>
class MpscFifo {
  public:
    explicit MpscFifo(
            MpscFifoConcurrency concurrency = MpscFifoConcurrency::MultipleProducers)
          : m_writeCount(0),
            // Initially no items have been written, so all (= capacity)
            // available items have been read and none are available.
            m_readCount(capacity),
            m_writeIndex(0),
            m_readIndex(0) {
        static_assert(capacity >= 1, "capacity too low");
        static_assert((capacity + 1) > 0, "capacity too high");
        if (concurrency == MpscFifoConcurrency::MultipleProducers) {
            m_writeMutex = std::make_unique<QMutex>();
        }
    }

    // Writers from multiple threads may enqueue items concurrently.
    // The argument is passed by value, because it is consumed by
    // this operation on success. The situation that the queue is
    // full and the operation fails by returning false is not expected
    // to happen frequently.
    bool enqueue(T value) {
        if (m_writeCount.fetchAndAddAcquire(1) >= capacity) {
            // No slots available for writing -> Undo changes and abort
            m_writeCount.fetchAndAddRelease(-1);
            return false;
        }
        {
            QMutexLocker locked(m_writeMutex.get());
            DEBUG_ASSERT(m_writeIndex >= 0);
            DEBUG_ASSERT(m_writeIndex <= capacity);
            m_buffer[m_writeIndex] = std::move(value);
            m_writeIndex = nextIndex(m_writeIndex);
        }
        // Finally allow the reader to access the enqueued buffer slot
        m_readCount.fetchAndAddRelease(-1);
        return true;
    }

    // Only a single reader at a time is allowed to dequeue items.
    // TODO(C++17): Use std::optional<T> as the return value
    bool dequeue(T* value) {
        if (m_readCount.fetchAndAddAcquire(1) >= capacity) {
            // No slots available for reading -> Undo changes and abort
            m_readCount.fetchAndAddRelease(-1);
            return false;
        }
        DEBUG_ASSERT(m_readIndex >= 0);
        DEBUG_ASSERT(m_readIndex <= capacity);
        *value = std::move(m_buffer[m_readIndex]);
        m_readIndex = nextIndex(m_readIndex);
        // Finally allow writers to overwrite the dequeued buffer slot
        m_writeCount.fetchAndAddRelease(-1);
        return true;
    }

  private:
    static int nextIndex(int index) {
        return (index + 1) % (capacity + 1);
    }

    // One additional slot is needed to decouple writers from the single reader
    T m_buffer[capacity + 1];

    // Both writers and the reader have a different view on the utilization of
    // the queue. The writers and readers respectively use acquire or release
    // memory ordering semantics according to their role for lock-free
    // coordination.
    QAtomicInt m_writeCount;
    QAtomicInt m_readCount;

    // Only a single writer is allowed at a time. Otherwise stale reads may
    // occur if a writer is delayed while accessing m_buffer!
    std::unique_ptr<QMutex> m_writeMutex;

    int m_writeIndex;
    int m_readIndex;
};
