#include "widget/wnotification.h"

#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QWidget>

WNotification::WNotification(mixxx::Notification* pNotification, QWidget* parent)
        : QWidget(parent),
          m_pNotification(pNotification) {
    setObjectName("Notification");
    setAttribute(Qt::WA_StyledBackground);

    QVBoxLayout* pMainLayout = new QVBoxLayout();
    QHBoxLayout* pTopBarLayout = new QHBoxLayout();

    const QString timeStr = pNotification->lastUpdated().toLocalTime().toString(
            QLocale::system().timeFormat(QLocale::ShortFormat));
    const QString dateTimeStr = pNotification->lastUpdated().toLocalTime().toString(
            QLocale::system().dateTimeFormat(QLocale::LongFormat));
    QLabel* pTimeLabel = new QLabel(timeStr);
    pTimeLabel->setObjectName("TimeLabel");
    pTimeLabel->setToolTip(dateTimeStr);
    pTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    pTopBarLayout->addWidget(pTimeLabel);

    pTopBarLayout->addStretch(1);

    QPushButton* pCloseButton = new QPushButton("");
    pCloseButton->setObjectName("CloseButton");
    pCloseButton->setToolTip(tr("Close Notification"));
    pTopBarLayout->addWidget(pCloseButton);
    connect(pCloseButton, &QAbstractButton::clicked, this, &WNotification::closed);

    pMainLayout->addLayout(pTopBarLayout);

    QLabel* pTextLabel = new QLabel(pNotification->text());
    pTextLabel->setObjectName("TextLabel");
    pTextLabel->setWordWrap(true);
    pMainLayout->addWidget(pTextLabel);

    const auto& buttons = pNotification->buttons();
    if (!buttons.isEmpty()) {
        QDialogButtonBox* pButtonBox = new QDialogButtonBox();
        for (const auto& button : buttons) {
            QPushButton* pButton = pButtonBox->addButton(
                    button.label, QDialogButtonBox::AcceptRole);
            connect(pButton, &QAbstractButton::clicked, button.receiver, button.func);
        }
        pMainLayout->addWidget(pButtonBox);
    }

    setLayout(pMainLayout);
}
