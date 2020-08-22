/// Utilities for iterating through a selection or collection
/// of tracks.

#pragma once

#include <QModelIndex>

#include "track/trackiterator.h"

class TrackCollection;

namespace mixxx {

/// Iterate over selected and valid(!) track pointers in a TrackModel.
/// Invalid (= nullptr) track pointers are skipped silently.
class TrackByIdCollectionIterator final
        : public virtual TrackPointerIterator {
  public:
    TrackByIdCollectionIterator(
            const TrackCollection* pTrackCollection,
            const TrackIdList& trackIds)
            : m_pTrackCollection(pTrackCollection),
              m_trackIdListIter(trackIds) {
        DEBUG_ASSERT(m_pTrackCollection);
    }
    ~TrackByIdCollectionIterator() override = default;

    void reset() override {
        m_trackIdListIter.reset();
    }

    std::optional<int> estimateItemsRemaining() override {
        return m_trackIdListIter.estimateItemsRemaining();
    }

    std::optional<TrackPointer> nextItem() override;

  private:
    const TrackCollection* const m_pTrackCollection;
    TrackIdListIterator m_trackIdListIter;
};

} // namespace mixxx
