#include "widget/wcuebutton.h"

#include <widget/hotcuedrag.h>

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>

#include "mixer/playerinfo.h"
#include "moc_wcuebutton.cpp"
#include "skin/legacy/skincontext.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/dnd.h"

using namespace mixxx::hotcuedrag;

WCueButton::WCueButton(QWidget* pParent, const QString& group)
        : WPushButton(pParent),
          m_group(group) {
}

void WCueButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    // For dnd/swapping hotcues we use the rendered widget pixmap as dnd cursor.
    // Unfortnately the margin that constraints the bg color is not considered,
    // so we shrink the rect by custom margins.
    // TODO Turn this into a qproperty, set in qss
    bool okay = false;
    int dndMargin = context.selectInt(node, QStringLiteral("DndRectMargin"), &okay);
    if (okay && dndMargin > 0) {
        m_dndRectMargins = QMargins(dndMargin, dndMargin, dndMargin, dndMargin);
    }
}

void WCueButton::mouseMoveEvent(QMouseEvent* pEvent) {
    TrackPointer pTrack = PlayerInfo::instance().getTrackInfo(m_group);
    if (!pTrack) {
        return;
    }

    // TODO Initiate drag only if pressing this button actually starts cue preview?
    if (DragAndDropHelper::mouseMoveInitiatesDrag(pEvent)) {
        const TrackId id = pTrack->getId();
        VERIFY_OR_DEBUG_ASSERT(id.isValid()) {
            return;
        }
        QDrag* pDrag = new QDrag(this);
        HotcueDragInfo dragData(id, kMainCueIndex);
        auto mimeData = std::make_unique<QMimeData>();
        mimeData->setData(kDragMimeType, dragData.toByteArray());
        pDrag->setMimeData(mimeData.release());

        // Use the currently rendered button as dnd cursor
        // (incl. hover and pressed style).
        // Note: both grab() and render() use the pure rect(),
        // i.e. these render with sharp corners and qss 'border-radius'
        // is not visible in the drag image.
        const QPixmap currLook = grab(rect().marginsRemoved(m_dndRectMargins));
        pDrag->setDragCursor(currLook, Qt::MoveAction);

        m_dragging = true;
        pDrag->exec();
        m_dragging = false;

        // Release this button afterwards.
        // This prevents both the preview and the pressed state from getting stuck.
        QEvent leaveEv(QEvent::Leave);
        QApplication::sendEvent(this, &leaveEv);
    }
}
