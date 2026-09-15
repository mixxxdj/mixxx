#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstring>
#include <type_traits>

/// This is a single producer single consumer ring buffer that allows
/// to access historic (delayed) values
template<typename T, std::size_t RingSize>
    requires std::is_trivially_copyable_v<T>
class DelayRing {
  public:
    /// Push new data to the ring
    /// Must be called from a single thread only
    void push(const T& data) {
        size_t write = m_writeIndex.load(std::memory_order_relaxed);
        std::memcpy(&m_ring[write % RingSize], &data, sizeof(T));
        m_writeIndex.store(write + 1, std::memory_order_release);
        if (m_level.load(std::memory_order_relaxed) < RingSize) {
            m_level.fetch_add(1, std::memory_order_release);
        }
        return;
    }

    /// returns true on success and a delayed value via pData
    /// getAt(0) returns the most recent value
    /// getAt(1) returns the value that has been pushed before
    /// keep 'at' small compared to kRingSize to make a ring lap during the
    /// call of getAt() unlikely
    /// Must be called from a single thread only
    bool getAt(std::size_t at, T* pData) {
        // not enough data available
        if (m_level.load(std::memory_order_acquire) <= at) {
            return false;
        }

        size_t writeBefore;
        size_t writeAfter;
        do {
            // snapshot the published write count (acquire)
            writeBefore = m_writeIndex.load(std::memory_order_acquire);

            // compute read index (unsigned arithmetic handles wrap)
            const size_t read = writeBefore - 1 - at;

            // copy the payload (non-atomic)
            std::memcpy(pData, &m_ring[read % RingSize], sizeof(T));

            // re-check the published write count (acquire)
            writeAfter = m_writeIndex.load(std::memory_order_acquire);

            // loop condition below will retry only if producer could have overwritten the slot
        } while (writeAfter - writeBefore >= RingSize - at);

        // At this point the producer did not advance enough to have
        // overwritten the slot we copied. Extra guard: ensure level didn't
        // drop below 'at' while we retried.
        if (m_level.load(std::memory_order_relaxed) <= at) {
            return false;
        }
        return true;
    }

    /// Clears the ring, can be called from any thread
    void reset() {
        m_level.store(0, std::memory_order_release);
    }

  private:
    std::array<T, RingSize> m_ring;
    std::atomic<std::size_t> m_writeIndex{0};
    std::atomic<std::size_t> m_level{0};
};
