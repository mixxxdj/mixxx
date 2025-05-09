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
        setFocus(Qt::MouseFocusReason);
    } else if (pEvent->type() == QEvent::HoverLeave) {
        // Also explicitly clear focus. This is required when we have other
        // Q[Widget]Actions in the same menu which have a 'selected' but no
        // 'focus' property.
        clearFocus();
    } else if (pEvent->type() == QEvent::MouseButtonDblClick) {
        return true;
    }
    return QCheckBox::eventFilter(pObj, pEvent);
}
