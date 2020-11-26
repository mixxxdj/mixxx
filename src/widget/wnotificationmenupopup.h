#pragma once

#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include "util/widgethelper.h"

namespace mixxx {
class NotificationManager;
}

class WNotificationMenuPopup : public QWidget {
    Q_OBJECT
  public:
    WNotificationMenuPopup(mixxx::NotificationManager* pNotificationManager,
            QWidget* parent = nullptr);

    ~WNotificationMenuPopup() {
    }

    void popup(const QPoint& p) {
        auto parentWidget = qobject_cast<QWidget*>(parent());
        QPoint topLeft = mixxx::widgethelper::mapPopupToScreen(*parentWidget, p, size());
        move(topLeft);
        show();
    }

  signals:
    void aboutToHide();
    void aboutToShow();

  public slots:
    void slotUpdateNotifications();

  protected:
    void closeEvent(QCloseEvent* event) override;

  private slots:
    void slotNotificationWidgetClosed();

  private:
    mixxx::NotificationManager* m_pNotificationManager;
    QScrollArea* m_pScrollArea;
    QVBoxLayout* m_pNotificationLayout;
};
