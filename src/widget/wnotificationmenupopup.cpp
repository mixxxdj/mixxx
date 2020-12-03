#include "widget/wnotificationmenupopup.h"

#include <QLabel>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "control/controlproxy.h"
#include "notification.h"
#include "notificationmanager.h"
#include "widget/wnotification.h"

namespace {
constexpr int kLayoutPageEmpty = 0;
constexpr int kLayoutPageLoading = 1;
constexpr int kLayoutPageNotifications = 2;
constexpr int kLayoutNumPages = 3;
} // namespace

WNotificationMenuPopup::WNotificationMenuPopup(
        mixxx::NotificationManager* pNotificationManager, QWidget* parent)
        : QWidget(parent), m_pNotificationManager(pNotificationManager) {
    connect(m_pNotificationManager,
            &mixxx::NotificationManager::notificationAdded,
            this,
            &WNotificationMenuPopup::slotUpdateNotifications);
    connect(m_pNotificationManager,
            &mixxx::NotificationManager::notificationClosed,
            this,
            &WNotificationMenuPopup::slotUpdateNotifications);

    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setContentsMargins(0, 0, 0, 0);

    QStackedLayout* pMainLayout = new QStackedLayout();
    pMainLayout->setSizeConstraint(QLayout::SetFixedSize);

    // No notifications page
    QLabel* pEmptyLabel = new QLabel(tr("The notification list is empty."), this);
    pEmptyLabel->setObjectName("NotificationsEmptyLabel");
    pEmptyLabel->setWordWrap(true);
    pMainLayout->addWidget(pEmptyLabel);

    // Loading page
    QLabel* pLoadingLabel = new QLabel(tr("Loading notifications..."), this);
    pLoadingLabel->setObjectName("NotificationsLoadingLabel");
    pLoadingLabel->setWordWrap(true);
    pMainLayout->addWidget(pLoadingLabel);

    // Notifications page
    QWidget* pWidget = new QWidget(this);
    pWidget->setObjectName("NotificationContainer");
    pWidget->setContentsMargins(0, 0, 0, 0);
    m_pNotificationLayout = new QVBoxLayout();
    pWidget->setLayout(m_pNotificationLayout);
    pMainLayout->addWidget(pWidget);

    DEBUG_ASSERT(pMainLayout->count() == kLayoutNumPages);
    setLayout(pMainLayout);
    // we need to update the the layout here since the size is used to
    // calculate the positioning later
    layout()->update();
    layout()->activate();
}

void WNotificationMenuPopup::slotUpdateNotifications() {
    const QList<mixxx::NotificationPointer> notifications =
            m_pNotificationManager->activeNotifications();

    auto pMainLayout = static_cast<QStackedLayout*>(layout());
    VERIFY_OR_DEBUG_ASSERT(pMainLayout) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(pMainLayout->count() == kLayoutNumPages) {
        return;
    }

    // No notifications, switch to page 1 of stacked layout.
    if (notifications.isEmpty()) {
        if (pMainLayout->currentIndex() != 0) {
            pMainLayout->setCurrentIndex(kLayoutPageEmpty);
            hide();
        }
        return;
    }

    pMainLayout->setCurrentIndex(kLayoutPageLoading);

    VERIFY_OR_DEBUG_ASSERT(m_pNotificationLayout) {
        return;
    }

    // Clear items from layout
    for (int i = m_pNotificationLayout->count() - 1; i >= 0; i--) {
        QLayoutItem* item = m_pNotificationLayout->itemAt(i);
        if (item->widget()) {
            item->widget()->hide();
        }
        m_pNotificationLayout->removeItem(item);
        delete item;
    }

    // Add items from layout
    for (const auto& pNotification : notifications) {
        WNotification* pNotificationWidget = new WNotification(pNotification);
        m_pNotificationLayout->addWidget(pNotificationWidget);
        connect(pNotificationWidget,
                &WNotification::closed,
                this,
                &WNotificationMenuPopup::slotNotificationWidgetClosed);
    }

    pMainLayout->setCurrentIndex(kLayoutPageNotifications);
    layout()->update();
    layout()->activate();
}

void WNotificationMenuPopup::closeEvent(QCloseEvent* event) {
    emit aboutToHide();
    QWidget::closeEvent(event);
}

void WNotificationMenuPopup::slotNotificationWidgetClosed() {
    WNotification* pNotificationWidget = qobject_cast<WNotification*>(sender());
    VERIFY_OR_DEBUG_ASSERT(pNotificationWidget) {
        return;
    }
    m_pNotificationManager->closeNotification(pNotificationWidget->notification());
}
