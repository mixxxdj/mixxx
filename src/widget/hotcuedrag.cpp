#include <widget/hotcuedrag.h>

#include <QDragEnterEvent>
#include <QMouseEvent>

#include "mixer/playerinfo.h"
#include "track/track.h"

namespace mixxx {

namespace hotcuedrag {

/// Check if the event is a valid hotcue drag or drop event.
/// Event must be a QDragEnterEvent or a QDropEvent.
/// In case of QDropEvent, the HotcueDragInfo is extracted and the pointer
/// data is used by the caller (WHotcueButton) to swap hotuces.
template<typename T>
bool isValidHotcueDragOrDropEvent(T* pEvent,
        const QString& group,
        QObject* pTarget = nullptr,
        int ignoreHotcueIndex = Cue::kNoHotCue,
        HotcueDragInfo* pDragData = nullptr) {
    constexpr bool isDrag = std::same_as<QDragEnterEvent, T>;
    // Allow source == target in the drag case so we get the drag cursor
    // (i.e. not the 'drop rejected' cursor) when starting the drag and when
    // dragging over the source later on.
    // Same exception for hotcue index check below.
    if (!isDrag && pEvent->source() == pTarget) {
        return false;
    }
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(group);
    if (!pTrack) {
        return false;
    }
    const QByteArray mimeDataBytes = pEvent->mimeData()->data(kDragMimeType);
    if (mimeDataBytes.isEmpty()) {
        return false;
    }
    const HotcueDragInfo dragData = HotcueDragInfo::fromByteArray(mimeDataBytes);
    if (dragData.isValid() &&
            (isDrag || dragData.hotcue != ignoreHotcueIndex) &&
            dragData.trackId == pTrack->getId()) {
        if (pDragData != nullptr) {
            *pDragData = dragData;
        }
        return true;
    }
    return false;
};

bool isValidHotcueDragEvent(QDragEnterEvent* pEvent,
        const QString& group,
        int ignoreHotcueIndex) {
    return isValidHotcueDragOrDropEvent(pEvent, group, nullptr, ignoreHotcueIndex);
}

bool isValidHotcueDropEvent(QDropEvent* pEvent,
        const QString& group,
        QObject* pTarget,
        int ignoreHotcueIndex,
        HotcueDragInfo* pDragData) {
    return isValidHotcueDragOrDropEvent(pEvent, group, pTarget, ignoreHotcueIndex, pDragData);
}

} // namespace hotcuedrag

} // namespace mixxx
