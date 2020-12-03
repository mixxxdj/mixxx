#pragma once
#include <QDateTime>
#include <QList>
#include <QMultiMap>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QTimer>
#include <memory>

#include "notification.h"

class ControlObject;

namespace mixxx {

class NotificationManager : public QObject {
    Q_OBJECT

  public:
    NotificationManager();
    ~NotificationManager();

    void notify(NotificationPointer pNotification);

    QList<NotificationPointer> activeNotifications() {
        return m_notifications;
    }

    void setInhibitNotifications(bool value) {
        if (m_inhibitNotifications == value) {
            return;
        }
        m_inhibitNotifications = value;
        if (m_inhibitNotifications) {
            m_notificationsAddedWhileInhibited = false;
            m_notificationsClosedWhileInhibited = false;
        } else {
            if (m_notificationsClosedWhileInhibited) {
                emit notificationClosed();
            }
            if (m_notificationsAddedWhileInhibited) {
                emit notificationAdded();
            }
        }
    }

    void closeNotification(NotificationPointer pNotification);

  signals:
    void notificationAdded();
    void notificationClosed();

  private slots:
    void slotNotificationAdded();
    void slotShowChangeRequested(double value);
    void slotNumNotificationsChangeRequested(double numNotifications);
    void slotUpdateNotifications();

  private:
    QTimer m_timer;
    QList<NotificationPointer> m_notifications;
    QMutex m_mutex;

    bool m_inhibitNotifications;
    bool m_notificationsAddedWhileInhibited;
    bool m_notificationsClosedWhileInhibited;
    ControlObject* m_pStatusControl;
    ControlObject* m_pNumNotificationsControl;
    ControlObject* m_pShowControl;
};

} // namespace mixxx
