#pragma once

#include <QBuffer>
#include <QMimeData>
#include <QString>

#include "track/cue.h"
#include "track/trackid.h"

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
        return trackId.isValid() && hotcue != Cue::kNoHotCue;
    }

    TrackId trackId = TrackId();
    int hotcue = Cue::kNoHotCue;
};

/// These check if the event is a valid hotcue drag or drop event.
/// Event must be a QDragEnterEvent or a QDropEvent.
/// In case of QDropEvent, the HotcueDragInfo is extracted from the QDrag and
/// assigned to the passed pointer. It's used by the caller (WHotcueButton)
/// to swap hotuces.
bool isValidHotcueDragEvent(QDragEnterEvent* pEvent,
        const QString& group,
        int ignoreHotcueIndex = Cue::kNoHotCue);
bool isValidHotcueDropEvent(QDropEvent* pEvent,
        const QString& group,
        QObject* pTarget,
        int ignoreHotcueIndex = Cue::kNoHotCue,
        HotcueDragInfo* pDragData = nullptr);

} // namespace hotcuedrag

} // namespace mixxx
