#include "widget/wmenucheckbox.h"

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
    if (isEnabled() && pEvent->type() == QEvent::HoverEnter) {
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
