#include "widget/wnotificationmenupopup.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

WNotificationMenuPopup::WNotificationMenuPopup(QWidget* parent)
        : QWidget(parent) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WNotificationMenuPopup");

    QLabel* pLabel = new QLabel(this);
    pLabel->setText("Hello");

    QVBoxLayout* pMainLayout = new QVBoxLayout();
    pMainLayout->addWidget(pLabel);
    setLayout(pMainLayout);
    // we need to update the the layout here since the size is used to
    // calculate the positioning later
    layout()->update();
    layout()->activate();
}

void WNotificationMenuPopup::closeEvent(QCloseEvent* event) {
    emit aboutToHide();
    QWidget::closeEvent(event);
}
