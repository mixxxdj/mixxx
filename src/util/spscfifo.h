#pragma once

#include <atomic>

#include "util/assert.h"

// Based on the WeakRB (Weak consistency Ring Buffer) implementation
// proposed in this paper: https://www.di.ens.fr/~guatto/papers/sbac13.pdf
template<typename T>
class SpscFifo final {
  public:
    // The requested capacity is adjusted to the next power
    // of two.
    explicit SpscFifo(size_t capacity)
            : m_capacity(next_power_of_two(capacity)),
              m_data(new T[m_capacity]),
              m_front(0),
              m_back(0),
              m_pfront(0),
              m_cback(0) {
        DEBUG_ASSERT(m_capacity > 0);
        DEBUG_ASSERT(std::atomic_is_lock_free(&m_front));
        DEBUG_ASSERT(std::atomic_is_lock_free(&m_back));
    }
    SpscFifo(const SpscFifo&) = delete;
    SpscFifo(SpscFifo&&) = delete;
    ~SpscFifo() {
        delete[] m_data;
    }

    size_t capacity() const {
        return m_capacity;
    }

    bool push(T data) {
        return push(&data, 1);
    }
    bool push(T* data, size_t count) {
        auto back = m_back.load(std::memory_order_relaxed);
        if ((m_pfront + m_capacity) - back < count) {
            m_pfront = std::atomic_load_explicit(&m_front, std::memory_order_acquire);
            if ((m_pfront + m_capacity) - back < count) {
                return false;
            }
        }
        for (size_t i = 0; i < count; ++i) {
            m_data[(back + i) % m_capacity] = std::move(data[i]);
        }
        m_back.store(back + count, std::memory_order_release);
        return true;
    }

    bool pop(T* data, size_t count = 1) {
        auto front = m_front.load(std::memory_order_relaxed);
        if (m_cback - front < count) {
            m_cback = m_back.load(std::memory_order_acquire);
            if (m_cback - front < count) {
                return false;
            }
        }
        for (size_t i = 0; i < count; ++i) {
            data[i] = std::move(m_data[(front + i) % m_capacity]);
        }
        m_front.store(front + count, std::memory_order_release);
        return true;
    }

  private:
    static size_t next_power_of_two(size_t size) {
        DEBUG_ASSERT(size > 0);
        if (size <= 1) {
            return size;
        }
        size_t psize = 2;
        --size;
        while ((psize > 0) && (size >>= 1)) {
            psize <<= 1;
        }
        DEBUG_ASSERT(psize > 0);
        return psize;
    }

    const size_t m_capacity;
    T* const m_data;

    std::atomic<size_t> m_front;
    std::atomic<size_t> m_back;

    // Only accessed by producer
    size_t m_pfront;

    // Only accessed by consumer
    size_t m_cback;
};
