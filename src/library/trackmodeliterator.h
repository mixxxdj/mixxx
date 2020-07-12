/// Utilities for iterating through a selection or collection
/// of tracks identified by QModelIndex.

#pragma once

#include <QModelIndex>

#include "track/trackiterator.h"

class TrackModel;

namespace mixxx {

/// Iterate over selected, valid track ids in a TrackModel.
/// Invalid track ids are skipped silently.
class TrackIdModelIterator final
        : public virtual TrackIdIterator {
  public:
    TrackIdModelIterator(
            const TrackModel* pTrackModel,
            const QModelIndexList& indexList)
            : m_pTrackModel(pTrackModel),
              m_modelIndexListIter(indexList) {
        DEBUG_ASSERT(m_pTrackModel);
    }
    ~TrackIdModelIterator() override = default;

    void reset() override {
        m_modelIndexListIter.reset();
    }

    std::optional<int> estimateItemsRemaining() override {
        return m_modelIndexListIter.estimateItemsRemaining();
    }

    std::optional<TrackId> nextItem() override;

  private:
    const TrackModel* const m_pTrackModel;
    ListItemIterator<QModelIndex> m_modelIndexListIter;
};

/// Iterate over selected, valid track pointers in a TrackModel.
/// Invalid (= nullptr) track pointers are skipped silently.
class TrackPointerModelIterator final
        : public virtual TrackPointerIterator {
  public:
    TrackPointerModelIterator(
            const TrackModel* pTrackModel,
            const QModelIndexList& indexList)
            : m_pTrackModel(pTrackModel),
              m_modelIndexListIter(indexList) {
        DEBUG_ASSERT(m_pTrackModel);
    }
    ~TrackPointerModelIterator() override = default;

    void reset() override {
        m_modelIndexListIter.reset();
    }

    std::optional<int> estimateItemsRemaining() override {
        return m_modelIndexListIter.estimateItemsRemaining();
    }

    std::optional<TrackPointer> nextItem() override;

  private:
    const TrackModel* const m_pTrackModel;
    ListItemIterator<QModelIndex> m_modelIndexListIter;
};

} // namespace mixxx
