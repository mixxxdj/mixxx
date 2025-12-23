/// Utilities for iterating through a selection or collection
/// of tracks.

#pragma once

#include "track/trackiterator.h"

class TrackCollectionManager;

namespace mixxx {

/// Iterate over selected and valid(!) track pointers in a TrackModel.
/// Invalid (= nullptr) track pointers are skipped silently.
class TrackByIdCollectionIterator final
        : public virtual TrackPointerIterator {
  public:
    TrackByIdCollectionIterator(
            const TrackCollectionManager* pTrackCollectionManager,
            const TrackIdList& trackIds)
            : m_pTrackCollectionManager(pTrackCollectionManager),
              m_trackIdListIter(trackIds) {
        DEBUG_ASSERT(m_pTrackCollectionManager);
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
    const TrackCollectionManager* const m_pTrackCollectionManager;
    TrackIdListIterator m_trackIdListIter;
};

} // namespace mixxx
