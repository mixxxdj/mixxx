#pragma once

#include <atomic>
#include <limits>

#include "util/assert.h"
#include "util/math.h"

// Based on the WeakRB (Weak consistency Ring Buffer) implementation
// proposed in these papers:
//
// https://www.di.ens.fr/~guatto/papers/sbac13.pdf
// https://hal.archives-ouvertes.fr/tel-01709813v2/document
//
// The implementation contains many (sometimes trivial obvious)
// debug assertions to ensure that nothing gets out of control!
template<typename T, typename S = size_t>
class SpscFifo final {
  public:
    typedef T value_type;
    typedef S size_type;

    explicit SpscFifo(size_type capacity)
            : m_size(adjust_capacity(capacity) + 1),
              m_data(new T[m_size]),
              m_front(0),
              m_back(0),
              m_pfront(0),
              m_cback(0) {
        DEBUG_ASSERT(m_size > capacity);
        DEBUG_ASSERT(m_size < m_size + capacity); // no overflow
        DEBUG_ASSERT(std::atomic_is_lock_free(&m_front));
        DEBUG_ASSERT(std::atomic_is_lock_free(&m_back));
    }
    SpscFifo(const SpscFifo&) = delete;
    SpscFifo(SpscFifo&&) = delete;
    ~SpscFifo() {
        delete[] m_data;
    }

    size_type capacity() const {
        return m_size - 1;
    }

    ///////////////////////////////////////////////////////////////////
    // Producer operations
    ///////////////////////////////////////////////////////////////////

    // Check how many elements are immediately available for reading.
    // With min_count = 0 only relaxed memory ordering is applied
    // when accessing atomic values.
    size_type push_peek(size_type min_count = 0) const {
        return acquire_writable(min_count);
    }

    // Non-blocking skip write operation, i.e. only advance the
    // write position
    size_type push_skip(size_type count = 1) {
        auto writable = math_min(acquire_writable(count), count);
        if (writable > 0) {
            DEBUG_ASSERT(m_pfront < m_size);
            DEBUG_ASSERT(writable < m_size);
            DEBUG_ASSERT(m_pfront < m_pfront + writable); // no overflow
            m_pfront = (m_pfront + writable) % m_size;
            DEBUG_ASSERT(m_pfront < m_size);
            m_front.store(m_pfront, std::memory_order_release);
        }
        DEBUG_ASSERT(writable <= count);
        return writable;
    }

    // Non-blocking write operation (move)
    size_type push(value_type* data, size_type count = 1) {
        DEBUG_ASSERT(data || !count);
        auto writable = math_min(acquire_writable(count), count);
        if (writable > 0) {
            DEBUG_ASSERT(m_pfront < m_size);
            DEBUG_ASSERT(writable < m_size);
            for (size_type i = 0; i < writable; ++i) {
                m_data[(m_pfront + i) % m_size] = std::move(data[i]);
            }
            DEBUG_ASSERT(m_pfront < m_pfront + writable); // no overflow
            m_pfront = (m_pfront + writable) % m_size;
            DEBUG_ASSERT(m_pfront < m_size);
            m_front.store(m_pfront, std::memory_order_release);
        }
        DEBUG_ASSERT(writable <= count);
        return writable;
    }

    // Non-blocking write operation (copy)
    size_type push(const value_type* data, size_type count = 1) {
        DEBUG_ASSERT(data || !count);
        auto writable = math_min(acquire_writable(count), count);
        if (writable > 0) {
            DEBUG_ASSERT(m_pfront < m_size);
            DEBUG_ASSERT(writable < m_size);
            for (size_type i = 0; i < writable; ++i) {
                m_data[(m_pfront + i) % m_size] = data[i];
            }
            DEBUG_ASSERT(m_pfront < m_pfront + writable); // no overflow
            m_pfront = (m_pfront + writable) % m_size;
            DEBUG_ASSERT(m_pfront < m_size);
            m_front.store(m_pfront, std::memory_order_release);
        }
        DEBUG_ASSERT(writable <= count);
        return writable;
    }

    // Blocking write operation (move) that loops until all provided data
    // has been written
    void push_all(value_type* data, size_type count = 1) {
        size_type write_count = 0;
        while (write_count < count) {
            write_count += push(data + write_count, count - write_count);
        }
        DEBUG_ASSERT(write_count == count);
    }

    // Blocking write operation (copy) that loops until all provided data
    // has been written
    void push_all(const value_type* data, size_type count = 1) {
        size_type write_count = 0;
        while (write_count < count) {
            write_count += push(data + write_count, count - write_count);
        }
        DEBUG_ASSERT(write_count == count);
    }

    // Convenience method for writing a single element by value
    bool try_push(value_type data) {
        return push(&data, 1) == 1;
    }

    ///////////////////////////////////////////////////////////////////
    // Consumer operations
    ///////////////////////////////////////////////////////////////////

    // Check how many elements are immediately available for reading.
    // With min_count = 0 only relaxed memory ordering is applied
    // when accessing atomic values.
    size_type pop_peek(size_type min_count = 0) const {
        return acquire_readable(min_count);
    }

    // Non-blocking skip read operation, i.e. only advance the
    // read position
    size_type pop_skip(size_type count = 1) {
        auto readable = math_min(acquire_readable(count), count);
        if (readable > 0) {
            DEBUG_ASSERT(m_cback < m_size);
            DEBUG_ASSERT(readable < m_size);
            DEBUG_ASSERT(m_cback < m_cback + readable); // no overflow
            m_cback = (m_cback + readable) % m_size;
            DEBUG_ASSERT(m_cback < m_size);
            m_back.store(m_cback, std::memory_order_release);
        }
        DEBUG_ASSERT(readable <= count);
        return readable;
    }

    // Non-blocking read operation
    size_type pop(value_type* data, size_type count = 1) {
        DEBUG_ASSERT(data || !count);
        auto readable = math_min(acquire_readable(count), count);
        if (readable > 0) {
            DEBUG_ASSERT(m_cback < m_size);
            DEBUG_ASSERT(readable < m_size);
            for (size_type i = 0; i < readable; ++i) {
                data[i] = std::move(m_data[(m_cback + i) % m_size]);
            }
            DEBUG_ASSERT(m_cback < m_cback + readable); // no overflow
            m_cback = (m_cback + readable) % m_size;
            DEBUG_ASSERT(m_cback < m_size);
            m_back.store(m_cback, std::memory_order_release);
        }
        DEBUG_ASSERT(readable <= count);
        return readable;
    }

    // Blocking read operation that loops until all requested data
    // has been read
    void pop_all(value_type* data, size_type count = 1) {
        size_type read_count = 0;
        while (read_count < count) {
            read_count += pop(data + read_count, count - read_count);
        }
        DEBUG_ASSERT(read_count == count);
    }

  private:
    static size_type adjust_capacity(size_type capacity) {
        VERIFY_OR_DEBUG_ASSERT(capacity <= std::numeric_limits<size_type>::max() / 2) {
            // Max. capacity exceeded - prevent overflow during push/pop operations!
            capacity = std::numeric_limits<size_type>::max() / 2;
        }
        VERIFY_OR_DEBUG_ASSERT(capacity > 0) {
            // An empty FIFO works, but it would be useless
            capacity = 1;
        }
        return capacity;
    }

    size_type min_writable(size_type back) const {
        return (back + (m_size - 1) - m_pfront) % m_size;
    }

    size_type acquire_writable(size_type min_count) const {
        auto back = m_back.load(std::memory_order_relaxed);
        DEBUG_ASSERT(back < m_size);
        auto writable = min_writable(back);
        if (writable < min_count) {
            m_pfront = m_front.load(std::memory_order_acquire);
            DEBUG_ASSERT(m_pfront < m_size);
            DEBUG_ASSERT(writable <= min_writable(back));
            writable = min_writable(back);
        }
        DEBUG_ASSERT(writable < m_size);
        return writable;
    }

    size_type min_readable(size_type front) const {
        return (front + m_size - m_cback) % m_size;
    }

    size_type acquire_readable(size_type min_count) const {
        auto front = m_front.load(std::memory_order_relaxed);
        DEBUG_ASSERT(front < m_size);
        auto readable = min_readable(front);
        if (readable < min_count) {
            m_cback = m_back.load(std::memory_order_acquire);
            DEBUG_ASSERT(m_cback < m_size);
            DEBUG_ASSERT(readable <= min_readable(front));
            readable = min_readable(front);
        }
        DEBUG_ASSERT(readable < m_size);
        return readable;
    }

    // Internal buffer
    const size_type m_size;
    value_type* const m_data;

    // Shared write position
    std::atomic<size_type> m_front;

    // Shared read position
    std::atomic<size_type> m_back;

    // Cached write position for the producer
    mutable size_type m_pfront;

    // Cached read position for the consumer
    mutable size_type m_cback;
};
