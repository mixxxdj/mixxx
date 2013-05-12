
#ifndef CONTROLOBJECTBASE_H_
#define CONTROLOBJECTBASE_H_

#include <QAtomicInt>
#include <QObject>

// for look free access, this value has to be >= the number of value using threads
const int cReaderSlotCnt = 7;

template<typename T>
class ControlObjectRingValue {
  public:
    ControlObjectRingValue()
        : m_value(T()),
          m_readerSlots(cReaderSlotCnt) {
    }

    bool tryGet(T* value) {
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
    QAtomicInt m_readerSlots;
};

// Ring buffer based implementation for all Types sizeof(T) > sizeof(void*)
template<typename T, bool ATOMIC = false>
class ControlObjectValue {
  public:
    inline T getValue() {
        T value = T();
        unsigned int index = (unsigned int)m_readIndex
                % (cReaderSlotCnt + 1);
        while (m_ring[index].tryGet(&value) == false) {
            // We are here if
            // 1) there are more then cReaderSlotCnt reader (get) reading the same value or
            // 2) the formerly current value is locked by a writer
            // Case 1 is prevented by enough reader slots and should not happen
            // Case 2 happens when the a reader is delayed after reading the
            // m_currentIndex and the writers have written cReaderSlotCnt times so that the
            // formally current value is locked again for writing.
            // In both cases reading the less recent value will fix it.
            index = (index - 1) % (cReaderSlotCnt + 1);
        }
        return value;
    }

    inline void setValue(const T& value) {
        // Test if we can read atomic
        // This test is const and will be mad only at compile time
        unsigned int index;
        do {
            index = (unsigned int)m_writeIndex.fetchAndAddAcquire(1)
                    % (cReaderSlotCnt + 1);
            // This will be repeated if the value is locked
            // 1) by an other writer writing at the same time or
            // 2) a delayed reader is still blocking the formerly current value
            // In both cases writing to the next value will fix it.
        } while (!m_ring[index].trySet(value));
        m_readIndex = (int)index;
    }

  protected:
    ControlObjectValue()
        : m_readIndex(0),
          m_writeIndex(1) {
    }

  private:
    // In worst case, each reader can consume a reader slot from a different ring element.
    // In this case there is still one ring element available for writing.
    ControlObjectRingValue<T> m_ring[cReaderSlotCnt+1];
    QAtomicInt m_readIndex;
    QAtomicInt m_writeIndex;
};

// Specialized Template for types store able in a native atomic types
// It uses reinterpret_cast to store any value bit by bit into a QAtomicPointer
// container. reinterpret_cast guarantees that the value is unchanged if you
// cast it to a different type, and then reinterpret_cast it back to the
// original type.
template<typename T>
class ControlObjectValue<T, true> {
  public:
    inline T getValue() {
		return reinterpret_cast<T>(m_container);
    }

    inline void setValue(const T& value) {
        m_container = reinterpret_cast<void*>(value);
    }

  protected:
    ControlObjectValue()
        : m_container(reinterpret_cast<void*>(T())) {
    }

  private:
    QAtomicPointer<void*> m_container;
};

// This is a proxy Template to select the native atomic or the ring buffer
// Implementation depending on the target architecture
// Note: Qt does not support templates for signal and slots
// So the typified ControlObject has to handle the Event Queue connections
template<typename T>
class ControlObjectBase
    : public ControlObjectValue<T, sizeof(T) <= sizeof(void*)> {
  public:

    ControlObjectBase()
        : ControlObjectValue<T, sizeof(T) <= sizeof(void*)>() {
    }
};


#endif // CONTROLOBJECTBASE_H_

