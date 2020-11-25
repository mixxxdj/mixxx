#include "widget/wnotificationbutton.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

WNotificationButton::WNotificationButton(
        mixxx::NotificationManager* pNotificationManager, QWidget* pParent)
        : WPushButton(pParent), m_pNotificationManager(pNotificationManager) {
}

void WNotificationButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    m_pNotificationMenuPopup = make_parented<WNotificationMenuPopup>(this);
}

void WNotificationButton::slotShow() {
    m_pNotificationMenuPopup->popup(mapToGlobal(QPoint(0, height())));
}

void WNotificationButton::mousePressEvent(QMouseEvent* e) {
    slotShow();

    // Pass all other press events to the base class.
    WPushButton::mousePressEvent(e);
}
