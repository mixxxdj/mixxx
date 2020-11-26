#include "widget/wnotificationbutton.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "notificationmanager.h"

WNotificationButton::WNotificationButton(
        mixxx::NotificationManager* pNotificationManager, QWidget* pParent)
        : WPushButton(pParent),
          m_pNotificationManager(pNotificationManager),
          m_pNotificationMenuPopup(make_parented<WNotificationMenuPopup>(
                  m_pNotificationManager, this)) {
    connect(m_pNotificationManager,
            &mixxx::NotificationManager::notificationAdded,
            this,
            &WNotificationButton::slotShow);
}

void WNotificationButton::slotShow() {
    m_pNotificationMenuPopup->popup(mapToGlobal(QPoint(0, height())));
}

void WNotificationButton::moveEvent(QMoveEvent* event) {
    if (m_pNotificationMenuPopup->isVisible()) {
        slotShow();
    }

    WPushButton::moveEvent(event);
}
void WNotificationButton::mousePressEvent(QMouseEvent* event) {
    slotShow();

    WPushButton::mousePressEvent(event);
}
