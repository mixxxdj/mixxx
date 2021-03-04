#pragma once

#include <QDomNode>
#include <QMouseEvent>
#include <QString>
#include <QWidget>

#include "notification.h"
#include "util/parented_ptr.h"
#include "widget/wnotificationmenupopup.h"
#include "widget/wpushbutton.h"

class ControlProxy;

namespace mixxx {
class NotificationManager;
}

class WNotificationButton : public WPushButton {
    Q_OBJECT
  public:
    WNotificationButton(mixxx::NotificationManager* pNotificationManager, QWidget* pParent);
    ~WNotificationButton();

    void setup(const QDomNode& node, const SkinContext& context) override;

  protected:
    void moveEvent(QMoveEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

  private slots:
    void slotShowNotifications();
    void slotHideNotifications();
    void slotShowChanged(double value);

  private:
    mixxx::NotificationManager* m_pNotificationManager;
    parented_ptr<WNotificationMenuPopup> m_pNotificationMenuPopup;
    ControlProxy* m_pNotificationShowControlProxy;
};
