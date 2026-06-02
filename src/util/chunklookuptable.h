#pragma once

#include "util/assert.h"
#include "util/types.h"
#include <cstring>

// A fixed-size open-addressed hash table with linear probing.
// Designed for real-time safety: no allocations, no locks, O(1) average lookup.
// The type T must provide a `SINT getIndex() const` method.
template<typename T, int Size>
class ChunkLookupTable {
  public:
    ChunkLookupTable() {
        clear();
    }

    void clear() {
        std::memset(m_table, 0, sizeof(m_table));
    }

    void insert(SINT index, T* pItem) {
        DEBUG_ASSERT(pItem != nullptr);
        int pos = static_cast<int>(static_cast<uint>(index) % Size);
        int startPos = pos;
        while (m_table[pos]) {
            // The table should never be full.
            DEBUG_ASSERT(m_table[pos]->getIndex() != index);
            pos = (pos + 1) % Size;
            DEBUG_ASSERT(pos != startPos);
        }
        m_table[pos] = pItem;
    }

    T* lookup(SINT index) const {
        int pos = static_cast<int>(static_cast<uint>(index) % Size);
        while (m_table[pos]) {
            if (m_table[pos]->getIndex() == index) {
                return m_table[pos];
            }
            pos = (pos + 1) % Size;
        }
        return nullptr;
    }

    void remove(SINT index) {
        int pos = static_cast<int>(static_cast<uint>(index) % Size);
        while (m_table[pos]) {
            if (m_table[pos]->getIndex() == index) {
                m_table[pos] = nullptr;
                rehashCluster(pos);
                return;
            }
            pos = (pos + 1) % Size;
        }
    }

  private:
    void rehashCluster(int hole) {
        int next = (hole + 1) % Size;
        while (m_table[next]) {
            T* pItem = m_table[next];
            int idealPos = static_cast<int>(static_cast<uint>(pItem->getIndex()) % Size);

            // Check if next is not between idealPos and hole (circularly)
            bool between;
            if (idealPos <= next) {
                between = (idealPos <= hole && hole < next);
            } else {
                between = (idealPos <= hole || hole < next);
            }

            if (between) {
                m_table[hole] = pItem;
                m_table[next] = nullptr;
                hole = next;
            }
            next = (next + 1) % Size;
        }
    }

    T* m_table[Size];
};
