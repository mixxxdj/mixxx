#include "notificationmanager.h"

#include <QDebug>
#include <QMutexLocker>

namespace {

constexpr int kAutoTimeoutMillis = 5000;

}

namespace mixxx {

NotificationManager::NotificationManager() {
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &NotificationManager::slotUpdateNotifications);
};

void NotificationManager::notify(NotificationPointer pNotification) {
    if (!m_timer.isActive() && pNotification->flags().testFlag(NotificationFlag::AutoTimeout)) {
        m_timer.start();
    }

    qWarning() << "New notification:" << pNotification->text();

    QMutexLocker lock(&m_mutex);
    QDateTime dateTime = QDateTime::currentDateTime();
    m_activeNotifications.insert(dateTime, pNotification);
}

NotificationPointer NotificationManager::notify(const QString& text, NotificationFlags flags) {
    NotificationPointer pNotification = NotificationPointer(new Notification(text, flags));
    notify(pNotification);
    return pNotification;
}

void NotificationManager::slotUpdateNotifications() {
    QDateTime currentDateTime = QDateTime::currentDateTime();

    bool pendingNotifications = false;
    QMultiMap<QDateTime, NotificationPointer> outdatedNotifications;

    for (auto it = m_activeNotifications.constBegin();
            it != m_activeNotifications.constEnd();
            it++) {
        if (it.value()->flags().testFlag(NotificationFlag::AutoTimeout)) {
            if (it.key().msecsTo(currentDateTime) > kAutoTimeoutMillis) {
                m_inactiveNotifications.prepend(it.value());
                outdatedNotifications.insert(it.key(), it.value());
                qWarning() << "Notification timed out:" << it.value()->text();
            } else {
                pendingNotifications = true;
            }
        }
    }

    if (!outdatedNotifications.isEmpty()) {
        QMutexLocker lock(&m_mutex);
        for (auto it = outdatedNotifications.constBegin();
                it != outdatedNotifications.constEnd();
                it++) {
            m_activeNotifications.remove(it.key(), it.value());
        }
    }

    if (!pendingNotifications) {
        m_timer.stop();
    }
}

} // namespace mixxx
