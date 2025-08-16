#include <widget/hotcuedrag.h>

#include <QDragEnterEvent>
#include <QMouseEvent>

#include "mixer/playerinfo.h"
#include "track/track.h"

namespace mixxx {

namespace hotcuedrag {

template<typename T>
bool isValidHotcueDragOrDropEvent(T* pEvent,
        const QString& group,
        QObject* pTarget = nullptr,
        const QList<int>& ignoreIndices = QList<int>{Cue::kNoHotCue},
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
            // * WPlayButton dragEnter|drop accepts all hotcues and main cue
            // * WHotcueBbutton dragEnter accepts all hotcues incl. itself, rejects main cue
            // * WHotcueBbutton drop accepts all hotcues, rejects itself and main cue
            !ignoreIndices.contains(dragData.hotcue) &&
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
        const QList<int>& ignoreIndices) {
    return isValidHotcueDragOrDropEvent(pEvent, group, nullptr, ignoreIndices);
}

bool isValidHotcueDropEvent(QDropEvent* pEvent,
        const QString& group,
        QObject* pTarget,
        const QList<int>& ignoreIndices,
        HotcueDragInfo* pDragData) {
    return isValidHotcueDragOrDropEvent(pEvent, group, pTarget, ignoreIndices, pDragData);
}

} // namespace hotcuedrag

} // namespace mixxx
