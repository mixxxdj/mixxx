#include "widget/wnotificationbutton.h"

#include <QStyleOption>
#include <QStylePainter>
#include <QtDebug>

#include "control/control.h"
#include "notificationmanager.h"

WNotificationButton::WNotificationButton(
        mixxx::NotificationManager* pNotificationManager, QWidget* pParent)
        : WPushButton(pParent),
          m_pNotificationManager(pNotificationManager),
          m_pNotificationMenuPopup(make_parented<WNotificationMenuPopup>(
                  m_pNotificationManager, this)),
          m_pNotificationShowControlProxy(new ControlProxy(ConfigKey(
                  QStringLiteral("[Notifications]"), QStringLiteral("show")))) {
    m_pNotificationShowControlProxy->connectValueChanged(
            this, &WNotificationButton::slotShowChanged);
    connect(m_pNotificationMenuPopup,
            &WNotificationMenuPopup::aboutToHide,
            this,
            &WNotificationButton::slotHideNotifications);
}

WNotificationButton::~WNotificationButton() {
    delete m_pNotificationShowControlProxy;
}

void WNotificationButton::setup(const QDomNode& node, const SkinContext& context) {
    // Setup parent class.
    WPushButton::setup(node, context);

    auto pDisplayConnection = new ControlParameterWidgetConnection(
            this,
            ConfigKey("[Notifications]", "status"),
            nullptr,
            ControlParameterWidgetConnection::DIR_TO_WIDGET,
            ControlParameterWidgetConnection::EMIT_NEVER);
    addConnection(pDisplayConnection);
    setDisplayConnection(pDisplayConnection);

    QDomNode con = context.selectNode(node, QStringLiteral("Connection"));
    if (!con.isNull()) {
        SKIN_WARNING(node, context) << "Additional Connections are not allowed";
    }
}

void WNotificationButton::slotShowChanged(double value) {
    if (value == 0.0) {
        qWarning() << "Hiding Notifications";
        m_pNotificationMenuPopup->hide();
    } else {
        slotShowNotifications();
    }
}

void WNotificationButton::slotHideNotifications() {
    m_pNotificationShowControlProxy->set(0.0);
}

void WNotificationButton::slotShowNotifications() {
    qWarning() << "Showing Notifications";
    m_pNotificationMenuPopup->popup(mapToGlobal(QPoint(0, height())));
}

void WNotificationButton::moveEvent(QMoveEvent* event) {
    if (m_pNotificationShowControlProxy->toBool()) {
        slotShowNotifications();
    }

    WPushButton::moveEvent(event);
}
void WNotificationButton::mousePressEvent(QMouseEvent* event) {
    const double value = m_pNotificationShowControlProxy->toBool() ? 0.0 : 1.0;
    m_pNotificationShowControlProxy->set(value);

    WPushButton::mousePressEvent(event);
}
