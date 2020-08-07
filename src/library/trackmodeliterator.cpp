#include "library/trackmodeliterator.h"

#include "library/trackmodel.h"

namespace mixxx {

std::optional<TrackId> TrackIdModelIterator::nextItem() {
    const auto nextModelIndex =
            m_modelIndexListIter.nextItem();
    if (!nextModelIndex) {
        return std::nullopt;
    }
    const auto trackId =
            m_pTrackModel->getTrackId(*nextModelIndex);
    if (!trackId.isValid()) {
        return std::nullopt;
    }
    return std::make_optional(trackId);
}

std::optional<TrackPointer> TrackPointerModelIterator::nextItem() {
    const auto nextModelIndex =
            m_modelIndexListIter.nextItem();
    if (!nextModelIndex) {
        return std::nullopt;
    }
    const auto trackPtr =
            m_pTrackModel->getTrack(*nextModelIndex);
    if (!trackPtr) {
        return std::nullopt;
    }
    return std::make_optional(trackPtr);
}

} // namespace mixxx
