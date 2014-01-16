#include <QStyleOption>
#include <QStylePainter>

#include "widget/wbattery.h"

WBattery::WBattery(QWidget *parent)
        : WWidget(parent),
          m_pBattery(Battery::getBattery(this)) {
    if (m_pBattery) {
        connect(m_pBattery.data(), SIGNAL(stateChanged()),
                this, SLOT(update()));
    }
}

WBattery::~WBattery() {
}

void WBattery::setup(QDomNode node, const SkinContext& context) {
    // read the states which can be displayed
    // convert the QString to a sorted QList<int> of possible states
    QString statesString = context.selectString(node, "States");
    QStringList states = statesString.split(",", QString::SkipEmptyParts);
    QList<int> lStates;
    lStates.reserve(states.length());
    for (int i = 0; i < states.length(); i++) {
        lStates.append(states.at(i).toInt());
    }
    qSort(lStates.begin(), lStates.end());

    if (context.hasNode(node, "PixmapsCharging")) {
        QString sPathCharging = context.selectString(node, "PixmapsCharging");
        setPixmap(&m_qmPixmapsCharging, context.getSkinPath(sPathCharging),
                  lStates);
    }
    if (context.hasNode(node, "PixmapsDischarging")) {
        QString sPathDischarging = context.selectString(node, "PixmapsDischarging");
        setPixmap(&m_qmPixmapsDischarging,
                  context.getSkinPath(sPathDischarging), lStates);
    }
    if (context.selectBool(node, "HideWhenCharged", false)) {
        m_pPixmapCharged = m_qmPixmapsCharging[100];
    }

    if (m_pBattery) {
        m_pBattery->update();
    }
}

QString WBattery::getTimeLeft() {
    int minutes = m_pBattery ? m_pBattery->getMinutesLeft() : 0;
    if (minutes < 60) {
        return tr("%1 minutes").arg(minutes);
    }
    return tr("%1:%2").arg(minutes/60).arg(minutes%60, 2, 10, QChar('0'));
}

void WBattery::update() {
    Battery::ChargingState csChargingState = m_pBattery ?
            m_pBattery->getChargingState() : Battery::UNKNOWN;
    int iPercentage = m_pBattery ? m_pBattery->getPercentage() : 0;

    qDebug() << "WBattery: percentage:" << iPercentage;
    switch(csChargingState) {
        case Battery::CHARGING:
            m_pPixmap = m_qmPixmapsCharging[iPercentage];
            setToolTip(tr("Time until charged: %1").arg(getTimeLeft()));
            break;
        case Battery::DISCHARGING:
            m_pPixmap = m_qmPixmapsDischarging[iPercentage];
            setToolTip(tr("Time left: %1").arg(getTimeLeft()));
            break;
        case Battery::CHARGED:
            m_pPixmap = m_pPixmapCharged;
            setToolTip(tr("Battery fully charged."));
            break;
        case Battery::UNKNOWN:
        default:
            setToolTip(tr("Battery status unknown."));
    }
    // showEvent() hides the widget if m_pPixmap == 0, we have to show it again
    if (m_pPixmap)
        show();
    // call parent's update() to show changes, this should call QWidget::update()
    WWidget::update();
}

void WBattery::setPixmap(QMap<int, PaintablePointer>* target,
                         const QString& filename,
                         const QList<int>& chargeStates) {
    for (int i = 0; i < chargeStates.size(); ++i) {
        QString sPath = filename.arg(chargeStates.at(i), 3, 10, QChar('0'));
        PaintablePointer pPixmap = WPixmapStore::getPaintable(sPath);
        if (pPixmap) {
            setFixedSize(pPixmap->size());
            target->insert(chargeStates.at(i), pPixmap);
        }
    }
    // fill the QMap so every key 0-100 has a value
    for (int i = 0; i <= 100; ++i) {
        if (!(target->contains(i))) {
            QMap<int, PaintablePointer>::const_iterator next = target->upperBound(i);
            if (next != target->constEnd()) {
                target->insert(i, next.value());
            }
        }
    }
}

void WBattery::paintEvent(QPaintEvent*) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmap) {
        m_pPixmap->draw(0, 0, &p);
    }
}
