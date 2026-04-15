#include "widget/wplaybutton.h"

#include <QMouseEvent>

#include "control/controlobject.h"
#include "moc_wplaybutton.cpp"
#include "widget/hotcuedrag.h"

using namespace mixxx::hotcuedrag;

WPlayButton::WPlayButton(QWidget* pParent, const QString& group)
        : WPushButton(pParent),
          m_group(group) {
    setAcceptDrops(true);
}

void WPlayButton::dragEnterEvent(QDragEnterEvent* pEvent) {
    if (isValidHotcueDragEvent(pEvent, m_group)) {
        pEvent->acceptProposedAction();
    } else {
        pEvent->ignore();
    }
}

void WPlayButton::dropEvent(QDropEvent* pEvent) {
    // Enable 'play' if we drop a hotcue here while we're
    // previewing it but NOT playing.
    // If not previewing this is no-op.
    if (ControlObject::get(ConfigKey(m_group, QStringLiteral("play_latched"))) <= 0 &&
            isValidHotcueDropEvent(pEvent, m_group, this)) {
        // This seems counterintuitive but it's the same action that
        // allows to latch `play` with keyboard or controllers while
        // previewing a hotcue.
        // See EngineBuffer::slotControlPlayRequest()
        ControlObject::set(ConfigKey(m_group, QStringLiteral("play")), 0.0);
        pEvent->accept();
    } else {
        pEvent->ignore();
    }
}
