#include "widget/wtimerpopup.h"

#include <QHBoxLayout>
#include <QInputDialog>
#include <QVBoxLayout>


WTimerPopup::WTimerPopup(UserSettingsPointer pConfig, QWidget* parent)
        : QWidget(parent) {
    QWidget::hide();
    setWindowFlags(Qt::Popup);
    setAttribute(Qt::WA_StyledBackground);
    setObjectName("WTimerPopup");

    m_pTimerHeading = new QLabel(this);
    m_pTimerHeading->setToolTip(tr("Timer Header"));
    m_pTimerHeading->setObjectName("TimerHeaderLabel");
    m_pTimerHeading->setAlignment(Qt::AlignLeft);

    m_pStartButton = new QPushButton("", this);
    m_pStartButton->setToolTip(tr("Start"));
    m_pStartButton->setObjectName("TimerStartButton");
    connect(m_pStartButton, &QPushButton::clicked, this);

    m_pStopButton = new QPushButton("", this);
    m_pStopButton->setToolTip(tr("Stop"));
    m_pStopButton->setObjectName("TimerStopButton");
    connect(m_pStopButton, &QPushButton::clicked, this);
    // we need to update the the layout here since the size is used to
    // calculate the positioning later
    layout()->update();
    layout()->activate();
}