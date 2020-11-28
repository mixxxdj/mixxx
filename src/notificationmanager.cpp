#include "notificationmanager.h"

#include <QDebug>
#include <QMutexLocker>

#include "util/assert.h"

namespace {

constexpr int kAutoTimeoutSecs = 10;
}

namespace mixxx {

NotificationManager::NotificationManager()
        : m_inhibitNotifications(false),
          m_notificationsAddedWhileInhibited(false),
          m_notificationsClosedWhileInhibited(false) {
    m_timer.setInterval(1000);
    connect(&m_timer, &QTimer::timeout, this, &NotificationManager::slotUpdateNotifications);
};

void NotificationManager::notify(NotificationPointer pNotification) {
    VERIFY_OR_DEBUG_ASSERT(pNotification) {
        return;
    }
    qWarning() << "New notification:" << pNotification->text();

    if (!pNotification->lastUpdated().isValid()) {
        pNotification->setLastUpdated(QDateTime::currentDateTimeUtc());
    }

    if (!pNotification->flags().testFlag(NotificationFlag::Sticky)) {
        if (pNotification->timeoutSecs() == 0) {
            pNotification->setTimeoutSecs(kAutoTimeoutSecs);
        }

        if (!m_timer.isActive()) {
            m_timer.start();
        }
    }

    QMutexLocker lock(&m_mutex);
    m_notifications.prepend(pNotification);
    lock.unlock();

    if (m_inhibitNotifications) {
        m_notificationsAddedWhileInhibited = true;
    } else {
        emit notificationAdded();
    }
}

void NotificationManager::closeNotification(NotificationPointer pNotification) {
    m_notifications.removeAll(pNotification);
    if (m_inhibitNotifications) {
        m_notificationsClosedWhileInhibited = true;
    } else {
        emit notificationClosed();
    }
}

void NotificationManager::slotUpdateNotifications() {
    bool changed = false;
    bool timedNotificationLeft = false;

    QMutexLocker lock(&m_mutex);
    auto it = m_notifications.begin();
    while (it != m_notifications.end()) {
        const NotificationPointer pNotification = *it;
        if (pNotification->flags().testFlag(NotificationFlag::Sticky)) {
            it++;
            continue;
        }

        DEBUG_ASSERT(pNotification->lastUpdated().isValid());
        if (pNotification->lastUpdated().addSecs(
                    pNotification->timeoutSecs()) <=
                QDateTime::currentDateTimeUtc()) {
            qDebug() << "Notification timed out:" << pNotification->text();
            changed = true;
            it = m_notifications.erase(it);
        } else {
            timedNotificationLeft = true;
            it++;
        }
    }
    lock.unlock();

    if (!timedNotificationLeft) {
        m_timer.stop();
    }

    if (changed) {
        if (m_inhibitNotifications) {
            m_notificationsClosedWhileInhibited = true;
        } else {
            emit notificationClosed();
        }
    }
}

} // namespace mixxx
