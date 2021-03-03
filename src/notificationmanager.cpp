#include "notificationmanager.h"

#include <QDebug>
#include <QMutexLocker>

#include "control/controlobject.h"
#include "util/assert.h"

namespace {

constexpr int kAutoTimeoutSecs = 10;
}

namespace mixxx {

NotificationManager::NotificationManager()
        : m_inhibitNotifications(false),
          m_notificationsAddedWhileInhibited(false),
          m_notificationsClosedWhileInhibited(false),
          m_pStatusControl(
                  new ControlObject(ConfigKey(QStringLiteral("[Notifications]"),
                          QStringLiteral("status")))),
          m_pNumNotificationsControl(
                  new ControlObject(ConfigKey(QStringLiteral("[Notifications]"),
                          QStringLiteral("num_notifications")))),
          m_pShowControl(new ControlObject(ConfigKey(
                  QStringLiteral("[Notifications]"), QStringLiteral("show")))) {
    m_timer.setInterval(1000);
    m_pStatusControl->set(0);
    connect(&m_timer, &QTimer::timeout, this, &NotificationManager::slotUpdateNotifications);
    connect(this,
            &NotificationManager::notificationAdded,
            this,
            &NotificationManager::slotNotificationAdded);
    m_pNumNotificationsControl->connectValueChangeRequest(
            this, &NotificationManager::slotNumNotificationsChangeRequested);
    m_pShowControl->connectValueChangeRequest(this, &NotificationManager::slotShowChangeRequested);
    m_pStatusControl->connectValueChangeRequest(this, [this](double value) {
        if (value == 0.0 || value == 1.0 || value == 2.0) {
            m_pStatusControl->setAndConfirm(value);
        }
    });
};

NotificationManager::~NotificationManager() {
    delete m_pShowControl;
    delete m_pNumNotificationsControl;
    delete m_pStatusControl;
}

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

    m_pNumNotificationsControl->set(m_notifications.size());
    if (m_inhibitNotifications) {
        m_notificationsAddedWhileInhibited = true;
    } else {
        emit notificationAdded();
    }
}

void NotificationManager::slotNotificationAdded() {
    m_pShowControl->set(1.0);
}

void NotificationManager::closeNotification(NotificationPointer pNotification) {
    m_notifications.removeAll(pNotification);
    m_pNumNotificationsControl->set(m_notifications.size());
    if (m_inhibitNotifications) {
        m_notificationsClosedWhileInhibited = true;
    } else {
        emit notificationClosed();
    }
}

void NotificationManager::slotShowChangeRequested(double value) {
    const int numNotifications = static_cast<int>(m_pNumNotificationsControl->get());
    if (value == 0.0) {
        m_pShowControl->setAndConfirm(0.0);
        if (numNotifications > 0) {
            m_pStatusControl->set(1.0);
        } else {
            m_pStatusControl->set(0.0);
        }
        return;
    }

    if (numNotifications > 0) {
        m_pShowControl->setAndConfirm(1.0);
        m_pStatusControl->set(2.0);
    }
}

void NotificationManager::slotNumNotificationsChangeRequested(double numNotifications) {
    m_pNumNotificationsControl->setAndConfirm(numNotifications);

    if (numNotifications > 0) {
        if (m_pShowControl->toBool()) {
            m_pStatusControl->set(2.0);
        } else {
            m_pStatusControl->set(1.0);
        }
        return;
    }

    m_pShowControl->set(0.0);
    m_pStatusControl->set(0.0);
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
        m_pNumNotificationsControl->set(m_notifications.size());
        if (m_inhibitNotifications) {
            m_notificationsClosedWhileInhibited = true;
        } else {
            emit notificationClosed();
        }
    }
}

} // namespace mixxx
