#include "widget/wnotificationmenupopup.h"

#include <QLabel>
#include <QScrollArea>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QWidget>

#include "notification.h"
#include "notificationmanager.h"

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
    setFixedWidth(400);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum));

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
    auto* pLayout = new QVBoxLayout();
    pLayout->setSizeConstraint(QLayout::SetMinimumSize);
    pLayout->setContentsMargins(0, 0, 0, 0);
    pWidget->setLayout(pLayout);
    QScrollArea* pScrollArea = new QScrollArea(this);
    pScrollArea->setWidgetResizable(true);
    pScrollArea->setMaximumHeight(600);
    pScrollArea->setWidget(pWidget);
    pMainLayout->addWidget(pScrollArea);

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

    QLayoutItem* pItem = pMainLayout->itemAt(kLayoutPageNotifications);
    VERIFY_OR_DEBUG_ASSERT(pItem) {
        return;
    }

    QScrollArea* pScrollArea = static_cast<QScrollArea*>(pItem->widget());
    VERIFY_OR_DEBUG_ASSERT(pScrollArea) {
        return;
    }

    QWidget* pWidget = pScrollArea->widget();
    VERIFY_OR_DEBUG_ASSERT(pWidget) {
        return;
    }

    QLayout* pLayout = pWidget->layout();
    VERIFY_OR_DEBUG_ASSERT(pLayout) {
        return;
    }

    // Clear items from layout
    for (int i = 0; i < pLayout->count(); i++) {
        pLayout->removeItem(pLayout->itemAt(i));
    }

    // Add items from layout
    for (const auto& pNotification : notifications) {
        QLabel* pLabel = new QLabel(pNotification->text(), this);
        pLabel->setObjectName("Notification");
        pLabel->setWordWrap(true);
        pLayout->addWidget(pLabel);
    }

    pMainLayout->setCurrentIndex(kLayoutPageNotifications);
    layout()->update();
    layout()->activate();
}

void WNotificationMenuPopup::closeEvent(QCloseEvent* event) {
    emit aboutToHide();
    QWidget::closeEvent(event);
}
