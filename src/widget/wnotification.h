#pragma once

#include <QString>
#include <QWidget>

#include "notification.h"

class WNotification : public QWidget {
    Q_OBJECT
  public:
    WNotification(mixxx::NotificationPointer pNotificationManager, QWidget* parent = nullptr);
    ~WNotification() {
    }

    mixxx::NotificationPointer notification() {
        return m_pNotification;
    }

  signals:
    void closed();

  private:
    mixxx::NotificationPointer m_pNotification;
};
