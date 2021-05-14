#include "library/trackcollectioniterator.h"

#include "library/trackcollectionmanager.h"

namespace mixxx {

std::optional<TrackPointer> TrackByIdCollectionIterator::nextItem() {
    const auto nextTrackId =
            m_trackIdListIter.nextItem();
    if (!nextTrackId) {
        return std::nullopt;
    }
    const auto trackPtr =
            m_pTrackCollectionManager->getTrackById(*nextTrackId);
    if (!trackPtr) {
        return std::nullopt;
    }
    return std::make_optional(trackPtr);
}

} // namespace mixxx
