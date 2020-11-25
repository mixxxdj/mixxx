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

namespace mixxx {

class NotificationManager : public QObject {
    Q_OBJECT

  public:
    NotificationManager();
    ~NotificationManager() = default;

    void notify(NotificationPointer pNotification);

  private slots:
    void slotUpdateNotifications();

  private:
    QTimer m_timer;
    QMultiMap<QDateTime, NotificationPointer> m_timedNotifications;
    QList<NotificationPointer> m_stickyNotifications;
    QList<NotificationPointer> m_inactiveNotifications;
    QMutex m_mutex;
};

} // namespace mixxx
