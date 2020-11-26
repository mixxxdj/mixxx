#pragma once

#include <QDomNode>
#include <QMouseEvent>
#include <QString>
#include <QWidget>

#include "notification.h"
#include "util/parented_ptr.h"
#include "widget/wnotificationmenupopup.h"
#include "widget/wpushbutton.h"

namespace mixxx {
class NotificationManager;
}

class WNotificationButton : public WPushButton {
    Q_OBJECT
  public:
    WNotificationButton(mixxx::NotificationManager* pNotificationManager, QWidget* pParent);

  protected:
    void moveEvent(QMoveEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

  private slots:
    void slotShow();

  private:
    mixxx::NotificationManager* m_pNotificationManager;
    parented_ptr<WNotificationMenuPopup> m_pNotificationMenuPopup;
};
