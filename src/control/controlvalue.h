#pragma once

#include <limits>

#include <QAtomicInt>
#include <QObject>

#include "util/assert.h"

// for lock free access, this value has to be >= the number of value using threads
// value must be a fraction of an integer
constexpr int kDefaultRingSize = 8;
// there are basically unlimited readers allowed at each ring element
// but we have to count them so max() is just fine.
// NOTE(rryan): Wrapping max with parentheses avoids conflict with the max macro
// defined in windows.h.
constexpr int kMaxReaderSlots = (std::numeric_limits<int>::max)();

// A single instance of a value of type T along with an atomic integer which
// tracks the current number of readers or writers of the slot. The value
// m_readerSlots starts at kMaxReaderSlots and counts down to 0. If the value is
// 0 or less then reads to the value fail because there are either too many
// readers or a write is occurring. A write to the value will fail if
// m_readerSlots is not equal to kMaxReaderSlots (e.g. there is an active
// reader).
template <typename T>
class ControlRingValue {
  public:
    ControlRingValue()
      : m_readerSlots(kMaxReaderSlots) {
    }

    // Tries to copy the stored value if a reader slot is available.
    // This operation can be repeated multiple times for the same
    // slot, because the stored value is preserved.
    bool tryGet(T* value) const {
        // Read while consuming one readerSlot
        if (m_readerSlots.fetchAndAddAcquire(-1) > 0) {
            // Reader slot has been acquired, no writer is active
            *value = m_value;
            m_readerSlots.fetchAndAddRelease(1);
            // We need the early return here to make the compiler
            // aware that *value is initialised in the true case.
            return true;
        }
        m_readerSlots.fetchAndAddRelease(1);
        return false;
    }

    bool trySet(const T& value) {
        // try to lock this element entirely for reading
        if (m_readerSlots.testAndSetAcquire(kMaxReaderSlots, 0)) {
            m_value = value;
            // We need to re-add kMaxReaderSlots instead of storing it
            // to keep the balance if readers have decreased the number
            // of slots in the meantime!
            m_readerSlots.fetchAndAddRelease(kMaxReaderSlots);
            return true;
        }
        return false;
    }

  private:
    T m_value;
    mutable QAtomicInt m_readerSlots;
};

// Ring buffer based implementation for all Types sizeof(T) > sizeof(void*)

// An implementation of ControlValueAtomicBase for non-atomic types T. Uses a
// ring-buffer of ControlRingValues and a read pointer and write pointer to
// provide getValue()/setValue() methods which *sacrifice perfect consistency*
// for the benefit of wait-free read/write access to a value.
template <typename T, int cRingSize, bool ATOMIC = false>
class ControlValueAtomicBase {
  public:
    inline T getValue() const {
        T value;
        unsigned int index = static_cast<unsigned int>(m_readIndex.load()) % cRingSize;
        while (!m_ring[index].tryGet(&value)) {
            // We are here if
            // 1) there are more then kMaxReaderSlots reader (get) reading the same value or
            // 2) the formerly current value is locked by a writer
            // Case 1 does not happen because we have enough (0x7fffffff) reader slots.
            // Case 2 happens when the a reader is delayed after reading the
            // m_currentIndex and in the mean while a reader locks the formally current value
            // because it has written cRingSize times. Reading the less recent value will fix
            // it because it is now actually the current value.
            index = (index - 1) % cRingSize;
        }
        return value;
    }

    inline void setValue(const T& value) {
        // Test if we can read atomic
        // This test is const and will be mad only at compile time
        unsigned int index;
        do {
            index = static_cast<unsigned int>(m_writeIndex.fetchAndAddAcquire(1)) % cRingSize;
            // This will be repeated if the value is locked
            // 1) by another writer writing at the same time or
            // 2) a delayed reader is still blocking the formerly current value
            // In both cases writing to the next value will fix it.
        } while (!m_ring[index].trySet(value));
        m_readIndex = index;
    }

  protected:
    ControlValueAtomicBase() : m_readIndex(0), m_writeIndex(1) {
        // NOTE(rryan): Wrapping max with parentheses avoids conflict with the
        // max macro defined in windows.h.
        DEBUG_ASSERT(((std::numeric_limits<unsigned int>::max)() % cRingSize) == (cRingSize - 1));
    }

  private:
    // In worst case, each reader can consume a reader slot from a different ring element.
    // In this case there is still one ring element available for writing.
    ControlRingValue<T> m_ring[cRingSize];
    QAtomicInt m_readIndex;
    QAtomicInt m_writeIndex;
};

// Specialized template for types that are deemed to be atomic on the target
// architecture. Instead of using a read/write ring to guarantee atomicity,
// direct assignment/read of an aligned member variable is used.
template <typename T>
class ControlValueAtomicBase<T, true> {
  public:
    inline T getValue() const {
        return m_value;
    }

    inline T getValueOnce() {
        return std::move(m_value);
    }

    inline void setValue(const T& value) {
        m_value = value;
    }

  protected:
    ControlValueAtomicBase() = default;

  private:
#if defined(__GNUC__)
    T m_value __attribute__((aligned(sizeof(void*))));
#elif defined(_MSC_VER)
#ifdef _WIN64
    T __declspec(align(8)) m_value;
#else
    T __declspec(align(4)) m_value;
#endif
#else
    T m_value;
#endif
};

// ControlValueAtomic is a wrapper around ControlValueAtomicBase which uses the
// sizeof(T) to determine which underlying implementation of
// ControlValueAtomicBase to use. For types where sizeof(T) <= sizeof(void*),
// the specialized implementation of ControlValueAtomicBase for types that are
// atomic on the architecture is used.
template <typename T, int cRingSize = kDefaultRingSize>
class ControlValueAtomic : public ControlValueAtomicBase<T, cRingSize, sizeof(T) <= sizeof(void*)> {
  public:
    ControlValueAtomic() = default;
};
