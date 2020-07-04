/// Utilities for iterating over a collection or selection
/// of multiple items.

#pragma once

#include <QList>
#include <QVector>

#include "util/assert.h"

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

/// Generic class for iterating over QList.
template<typename T>
class ListItemIterator final
        : public virtual ItemIterator<T> {
  public:
    explicit ListItemIterator(
            const QList<T>& itemList)
            : m_itemList(itemList),
              m_nextIndex(0) {
    }
    ~ListItemIterator() override = default;

    void reset() override {
        m_nextIndex = 0;
    }

    std::optional<int> estimateItemsRemaining() override {
        DEBUG_ASSERT(m_nextIndex <= m_itemList.size());
        return std::make_optional(
                m_itemList.size() - m_nextIndex);
    }

    std::optional<T> nextItem() override {
        DEBUG_ASSERT(m_nextIndex <= m_itemList.size());
        if (m_nextIndex < m_itemList.size()) {
            return std::make_optional(m_itemList[m_nextIndex++]);
        } else {
            return std::nullopt;
        }
    }

  private:
    const QList<T> m_itemList;
    int m_nextIndex;
};

/// Generic class for iterating over QVector.
template<typename T>
class VectorItemIterator final
        : public virtual ItemIterator<T> {
  public:
    explicit VectorItemIterator(
            const QVector<T>& itemVector)
            : m_itemVector(itemVector),
              m_nextIndex(0) {
    }
    ~VectorItemIterator() override = default;

    void reset() override {
        m_nextIndex = 0;
    }

    std::optional<int> estimateItemsRemaining() override {
        DEBUG_ASSERT(m_nextIndex <= m_itemVector.size());
        return std::make_optional(
                m_itemVector.size() - m_nextIndex);
    }

    std::optional<T> nextItem() override {
        DEBUG_ASSERT(m_nextIndex <= m_itemVector.size());
        if (m_nextIndex < m_itemVector.size()) {
            return std::make_optional(m_itemVector[m_nextIndex++]);
        } else {
            return std::nullopt;
        }
    }

  private:
    const QVector<T> m_itemVector;
    int m_nextIndex;
};

} // namespace mixxx
