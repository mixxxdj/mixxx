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
    NotificationPointer notify(const QString& text,
            NotificationFlags flags = NotificationFlag::Default);

  private slots:
    void slotUpdateNotifications();

  private:
    QTimer m_timer;
    QMultiMap<QDateTime, NotificationPointer> m_activeNotifications;
    QList<NotificationPointer> m_inactiveNotifications;
    QMutex m_mutex;
};

} // namespace mixxx
