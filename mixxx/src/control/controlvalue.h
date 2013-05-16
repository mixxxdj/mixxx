#ifndef CONTROLVALUE_H
#define CONTROLVALUE_H

#include <limits>

#include <QAtomicInt>
#include <QObject>

// for look free access, this value has to be >= the number of value using threads
// value must be a fraction of an integer
const int cRingSize = 8;
// there are basicly unlimited readers allowed at each ring element
// but we have to count them so max() is just fine.
const int cReaderSlotCnt = std::numeric_limits<int>::max();

// A single instance of a value of type T along with an atomic integer which
// tracks the current number of readers or writers of the slot. The value
// m_readerSlots starts at cReaderSlotCnt and counts down to 0. If the value is
// 0 or less then reads to the value fail because there are either too many
// readers or a write is occurring. A write to the value will fail if
// m_readerSlots is not equal to cReaderSlotCnt (e.g. there is an active
// reader).
template<typename T>
class ControlRingValue {
  public:
    ControlRingValue()
        : m_value(T()),
          m_readerSlots(cReaderSlotCnt) {
    }

    bool tryGet(T* value) const {
        // Read while consuming one readerSlot
        bool hasSlot = (m_readerSlots.fetchAndAddAcquire(-1) > 0);
        if (hasSlot) {
            *value = m_value;
        }
        (void)m_readerSlots.fetchAndAddRelease(1);
        return hasSlot;
    }

    bool trySet(const T& value) {
        // try to lock this element entirely for reading
        if (m_readerSlots.testAndSetAcquire(cReaderSlotCnt, 0)) {
            m_value = value;
            m_readerSlots.fetchAndAddRelease(cReaderSlotCnt);
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
template<typename T, bool ATOMIC = false>
class ControlValueAtomicBase {
  public:
    inline T getValue() const {
        T value = T();
        unsigned int index = (unsigned int)m_readIndex
                % (cRingSize);
        while (m_ring[index].tryGet(&value) == false) {
            // We are here if
            // 1) there are more then cReaderSlotCnt reader (get) reading the same value or
            // 2) the formerly current value is locked by a writer
            // Case 1 does not happen because we have enough (0x7fffffff) reader slots.
            // Case 2 happens when the a reader is delayed after reading the
            // m_currentIndex and in the mean while a reader locks the formaly current value
            // because it has written cRingSize times. Reading the less recent value will fix
            // it because it is now actualy the current value.
            index = (index - 1) % (cRingSize);
        }
        return value;
    }

    inline void setValue(const T& value) {
        // Test if we can read atomic
        // This test is const and will be mad only at compile time
        unsigned int index;
        do {
            index = (unsigned int)m_writeIndex.fetchAndAddAcquire(1)
                    % (cRingSize);
            // This will be repeated if the value is locked
            // 1) by an other writer writing at the same time or
            // 2) a delayed reader is still blocking the formerly current value
            // In both cases writing to the next value will fix it.
        } while (!m_ring[index].trySet(value));
        m_readIndex = (int)index;
    }

  protected:
    ControlValueAtomicBase()
        : m_readIndex(0),
          m_writeIndex(1) {
        Q_ASSERT((std::numeric_limits<unsigned int>::max() % cRingSize) == (cRingSize - 1));
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
template<typename T>
class ControlValueAtomicBase<T, true> {
  public:
    inline T getValue() const {
        return m_value;
    }

    inline void setValue(const T& value) {
        m_value = value;
    }

  protected:
    ControlValueAtomicBase()
            : m_value(T()) {
    }

  private:
#if defined(__GNUC__)
    T m_value __attribute__ ((aligned(sizeof(void*))));
#elif defined(_MSC_VER)
    T __declspec(align(sizeof(void*))) m_value;
#else
    T m_value;
#endif
};

// ControlValueAtomic is a wrapper around ControlValueAtomicBase which uses the
// sizeof(T) to determine which underlying implementation of
// ControlValueAtomicBase to use. For types where sizeof(T) <= sizeof(void*),
// the specialized implementation of ControlValueAtomicBase for types that are
// atomic on the architecture is used.
template<typename T>
class ControlValueAtomic
    : public ControlValueAtomicBase<T, sizeof(T) <= sizeof(void*)> {
  public:

    ControlValueAtomic()
        : ControlValueAtomicBase<T, sizeof(T) <= sizeof(void*)>() {
    }
};

#endif /* CONTROLVALUE_H */
