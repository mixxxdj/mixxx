/// Utilities for iterating over a collection or selection
/// of multiple items.

#pragma once

#include <QList>
#include <QVector>

#include "util/assert.h"
#include "util/optional.h"

namespace mixxx {

/// A generic iterator interface.
///
/// The iterator needs to be resettable to allow repeated application.
template<typename T>
class ItemIterator {
  public:
    virtual ~ItemIterator() = default;

    /// Resets the iterator to the first position before starting a
    /// new iteration.
    ///
    /// This operation should be invoked regardless
    /// either if the iterator has been newly created or already
    /// been used for an preceding iteration.
    virtual void reset() = 0;

    /// Returns a best-effort guess of the number of items that
    /// are remaining for the iteration or std::nullopt if unknown.
    virtual std::optional<int> estimateItemsRemaining() = 0;

    /// Returns the next item or std::nullopt when done.
    virtual std::optional<T> nextItem() = 0;
};

/// Generic class for iterating over an indexed Qt collection
/// of known size.
template<typename T>
class IndexedCollectionIterator final
        : public virtual ItemIterator<typename T::value_type> {
  public:
    explicit IndexedCollectionIterator(
            const T& itemCollection)
            : m_itemCollection(itemCollection),
              m_nextIndex(0) {
    }
    ~IndexedCollectionIterator() override = default;

    void reset() override {
        m_nextIndex = 0;
    }

    std::optional<int> estimateItemsRemaining() override {
        DEBUG_ASSERT(m_nextIndex <= m_itemCollection.size());
        return std::make_optional(
                m_itemCollection.size() - m_nextIndex);
    }

    std::optional<typename T::value_type> nextItem() override {
        DEBUG_ASSERT(m_nextIndex <= m_itemCollection.size());
        if (m_nextIndex < m_itemCollection.size()) {
            return std::make_optional(m_itemCollection[m_nextIndex++]);
        } else {
            return std::nullopt;
        }
    }

  private:
    const T m_itemCollection;
    int m_nextIndex;
};

/// Generic class for iterating over QList.
template<typename T>
using ListItemIterator = IndexedCollectionIterator<QList<T>>;

/// Generic class for iterating over QVector.
template<typename T>
using VectorItemIterator = IndexedCollectionIterator<QVector<T>>;

} // namespace mixxx
