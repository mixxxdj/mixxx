#include "notificationmanager.h"

#include <QDebug>
#include <QMutexLocker>

#include "util/assert.h"

namespace {

constexpr int kAutoTimeoutMillis = 5000;

}

namespace mixxx {

NotificationManager::NotificationManager() {
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &NotificationManager::slotUpdateNotifications);
};

void NotificationManager::notify(NotificationPointer pNotification) {
    VERIFY_OR_DEBUG_ASSERT(pNotification) {
        return;
    }

    if (!m_timer.isActive() && pNotification->flags().testFlag(NotificationFlag::AutoTimeout)) {
        m_timer.start();
    }

    qWarning() << "New notification:" << pNotification->text();

    QMutexLocker lock(&m_mutex);
    if (pNotification->flags().testFlag(NotificationFlag::AutoTimeout)) {
        QDateTime timeoutDateTime = QDateTime::currentDateTime().addMSecs(kAutoTimeoutMillis);
        m_timedNotifications.insert(timeoutDateTime, pNotification);
    } else {
        m_stickyNotifications.prepend(pNotification);
    }
}

void NotificationManager::slotUpdateNotifications() {
    QDateTime currentDateTime = QDateTime::currentDateTime();

    auto it = m_timedNotifications.begin();
    while (it != m_timedNotifications.upperBound(QDateTime::currentDateTime())) {
        DEBUG_ASSERT(it.key().isValid());

        qWarning() << "Notification timed out:" << it.value()->text();
        m_inactiveNotifications.prepend(it.value());
        it = m_timedNotifications.erase(it);
    }

    if (m_timedNotifications.isEmpty()) {
        m_timer.stop();
    }
}

} // namespace mixxx
