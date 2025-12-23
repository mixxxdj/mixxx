#pragma once

#include <QBuffer>
#include <QMimeData>
#include <QString>

#include "track/cue.h"
#include "track/trackid.h"
#include "util/defs.h"

const QString kDragMimeType = QStringLiteral("hotcueDragInfo");

class TrackId;
class QDragEnterEvent;
class QDropEvent;

namespace mixxx {

namespace hotcuedrag {

struct HotcueDragInfo {
    constexpr HotcueDragInfo() {};
    constexpr HotcueDragInfo(TrackId id, int cue)
            : trackId(id),
              hotcue(cue) {};

    static HotcueDragInfo fromByteArray(const QByteArray& bytes) {
        QDataStream stream(bytes);
        TrackId trackId;
        int hotcue;
        stream >> trackId >> hotcue;
        return HotcueDragInfo(trackId, hotcue);
    };

    QByteArray toByteArray() const {
        QByteArray bytes;
        QDataStream dataStream(&bytes, QIODevice::WriteOnly);
        dataStream << trackId << hotcue;
        return bytes;
    };

    bool isValid() const {
        // We have hotcues 0..kMaxNumberOfHotcues-1, and the main cue (kMaxNumberOfHotcues)
        return trackId.isValid() && hotcue > Cue::kNoHotCue && hotcue <= kMaxNumberOfHotcues;
    }

    TrackId trackId = TrackId();
    int hotcue = Cue::kNoHotCue;
};

/// These check if the event is a valid hotcue or main cue drag or drop event.
/// Event must be a QDragEnterEvent or a QDropEvent.
/// Check if the event is a valid hotcue or main cue drag or drop event.
/// Event must be a QDragEnterEvent or a QDropEvent.
/// In case of QDropEvent onto WHotcueButton, the HotcueDragInfo is extracted
/// from the QDrag and assigned to the pointer. This data is used by the calling
/// WHotcueButton to swap hotcues.
bool isValidHotcueDragEvent(QDragEnterEvent* pEvent,
        const QString& group,
        const QList<int>& ignoreIndices = QList<int>{Cue::kNoHotCue});
bool isValidHotcueDropEvent(QDropEvent* pEvent,
        const QString& group,
        QObject* pTarget,
        const QList<int>& ignoreIndices = QList<int>{Cue::kNoHotCue},
        HotcueDragInfo* pDragData = nullptr);

} // namespace hotcuedrag

} // namespace mixxx
