#include "widget/wmenucheckbox.h"

#include <QApplication>
#include <QMouseEvent>

#include "moc_wmenucheckbox.cpp"

WMenuCheckBox::WMenuCheckBox(QWidget* pParent)
        : WMenuCheckBox(QString(), pParent) {
}

WMenuCheckBox::WMenuCheckBox(const QString& label, QWidget* pParent)
        : QCheckBox(label, pParent) {
    installEventFilter(this);
}

bool WMenuCheckBox::eventFilter(QObject* pObj, QEvent* pEvent) {
    if (pObj != this) {
        return QCheckBox::eventFilter(pObj, pEvent);
    }

    QKeyEvent* pKE;
    if (isEnabled() &&
            (pEvent->type() == QEvent::KeyPress || pEvent->type() == QEvent::KeyRelease) &&
            (pKE = static_cast<QKeyEvent*>(pEvent)) && pKE->key() != Qt::Key_Space) {
        // Forward key events (except Space!) to the parent. Otherwise
        // a) Enter/Return are swallowed by QAbstractButton (which QCheckBox inherits)
        // b) Up/Down are not correctly passed to the parent if the parent is a
        //    top-level QMenu (like the context menu of WTrackTableViewHeader)
        //    since the menu apparently does not get focus (QMenu::activeAction() = nullptr
        //    when Up/Down switch between QWidgetActions and their widgets get focus.
        // Only Space has to be processed by us (QAbstractButton) in order to toggle
        // check state and emit toggled() which the menu usually connects to.
        // Observed with Qt 6.4 and 6.10
        auto* pParent = pObj->parent();
        if (pParent) {
            QApplication::sendEvent(pParent, pEvent);
            return true;
        }
    } else if (isEnabled() && pEvent->type() == QEvent::HoverEnter) {
        // Highlights the entire parent QWidgetAction
        setFocus(Qt::MouseFocusReason);
    } else if (pEvent->type() == QEvent::HoverLeave) {
        // Also explicitly clear focus to remove the highlight. This is required
        // when we have other Q[Widget]Actions in the same menu which have a
        // 'selected' but no 'focus' property.
        clearFocus();
    } else if (isEnabled() && pEvent->type() == QEvent::MouseButtonRelease) {
        // Note: QCheckBox/QAbstractButton and QMenu act on release, not on click.
        // With the original QCheckBox/QAbstractButton in QWidgetAction implementation,
        // the click (release) on the area that is not the text and not the box
        // (indicator), the click would be forwarded to the parent (go figure..).
        // The QWidgetAction would then emit the `triggered()` signal which causes
        // the parent menu to close.
        // For consistency with clicks on the text or box, we need to prevent that.
        // Just toggle the box and swallow the event.
        toggle();
        return true;
    } else if (pEvent->type() == QEvent::MouseButtonDblClick) {
        return true;
    }
    return QCheckBox::eventFilter(pObj, pEvent);
}
