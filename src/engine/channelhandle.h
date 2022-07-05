#pragma once

#include <QVarLengthArray>

#include "control/grouphandle.h"

constexpr int kMaxExpectedChannelGroups = 256;

// An associative container mapping GroupHandle to a template type T. Backed
// by a QVarLengthArray with ChannelHandleMap::kMaxExpectedChannelGroups pre-allocated
// entries. Insertions are amortized O(1) time (if less than kMaxExpectedChannelGroups
// exist then no allocation will occur -- insertion is a mere copy). Lookups are
// O(1) and quite fast -- a simple index into an array using the handle's
// integer value.
template <class T>
class ChannelHandleMap {
    typedef QVarLengthArray<T, kMaxExpectedChannelGroups> container_type;

  public:
    typedef typename QVarLengthArray<T, kMaxExpectedChannelGroups>::const_iterator const_iterator;
    typedef typename QVarLengthArray<T, kMaxExpectedChannelGroups>::iterator iterator;

    ChannelHandleMap()
            : m_dummy{} {
    }

    const T& at(GroupHandle handle) const {
        const auto index = indexOfGroupHandle(handle);
        if (index < 0) {
            return m_dummy;
        }
        DEBUG_ASSERT(index < m_data.size());
        return m_data.at(index);
    }

    void insert(GroupHandle handle, const T& value) {
        const auto index = indexOfGroupHandle(handle);
        if (index < 0) {
            return;
        }
        maybeExpand(index + 1);
        m_data[index] = value;
    }

    T& operator[](GroupHandle handle) {
        const auto index = indexOfGroupHandle(handle);
        if (index < 0) {
            return m_dummy;
        }
        maybeExpand(index + 1);
        return m_data[index];
    }

    void clear() {
        m_data.clear();
    }

    typename container_type::iterator begin() {
        return m_data.begin();
    }

    typename container_type::const_iterator begin() const {
        return m_data.begin();
    }

    typename container_type::iterator end() {
        return m_data.end();
    }

    typename container_type::const_iterator end() const {
        return m_data.end();
    }

  private:
    inline void maybeExpand(int iSize) {
        if (QTypeInfo<T>::isComplex) {
            // The value for complex types is initialized by QVarLengthArray
            if (m_data.size() < iSize) {
                m_data.resize(iSize);
            }
        } else {
            // We need to initialize simple types ourselves
            while (m_data.size() < iSize) {
                m_data.append({});
            }
        }
    }
    container_type m_data;
    T m_dummy;
};
