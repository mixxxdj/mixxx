#include "widget/wbattery.h"
#include "widget/wpixmapstore.h"

WBattery::WBattery(QWidget *parent)
        : WWidget(parent),
          m_pPixmap(0),
          m_pPixmapCharged(0),
          m_qmPixmapsCharging(),
          m_qmPixmapsDischarging() {
    battery = Battery::getBattery(this);
    connect(battery, SIGNAL(stateChanged()),
            this, SLOT(update()));
}

WBattery::~WBattery() {
    delete battery;
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
        setPixmap(&m_qmPixmapsCharging, sPathCharging, lStates);
    }
    if (context.hasNode(node, "PixmapsDischarging")) {
        QString sPathDischarging = context.selectString(node, "PixmapsDischarging");
        setPixmap(&m_qmPixmapsDischarging, sPathDischarging, lStates);
    }
    if (context.selectBool(node, "HideWhenCharged", false)) {
        m_pPixmapCharged = m_qmPixmapsCharging[100];
    }

    battery->update();
}

QString WBattery::getTimeLeft() {
    int minutes = battery->getMinutesLeft();
    if (minutes < 60) {
        return QString("%1 minutes").arg(minutes);
    }
    return QString("%1:%2").arg(minutes/60).arg(minutes%60, 2, 10, QChar('0'));
}

void WBattery::update() {
    Battery::ChargingState csChargingState = battery->getChargingState();
    int iPercentage = battery->getPercentage();
    qDebug() << "WBattery: percentage:" << iPercentage;
    switch(csChargingState) {
        case Battery::CHARGING:
            m_pPixmap = m_qmPixmapsCharging[iPercentage];
            setToolTip("Time until charged: " + getTimeLeft());
            break;
        case Battery::DISCHARGING:
            m_pPixmap = m_qmPixmapsDischarging[iPercentage];
            setToolTip("Time left: " + getTimeLeft());
            break;
        case Battery::CHARGED:
            m_pPixmap = m_pPixmapCharged;
            setToolTip("Battery fully charged");
            break;
        case Battery::UNKNOWN:
        default:
            setToolTip("Battery status unknown");
    }
    // showEvent() hides the widget if m_pPixmap == 0, we have to show it again
    if (m_pPixmap) show();
    // call parent's update() to show changes, this should call QWidget::update()
    WWidget::update();
}

void WBattery::setPixmap(QMap<int, QPixmap*> *target, const QString &filename, const QList<int> &chargeStates) {
    for (int i = 0; i < chargeStates.size(); ++i) {
        QString sPath = filename.arg(chargeStates.at(i), 3, 10, QChar('0'));
        QPixmap *pPixmap = WPixmapStore::getPixmap(getPath(sPath));
        if (pPixmap) {
            setFixedSize(pPixmap->size());
            target->insert(chargeStates.at(i), pPixmap);
        }
    }
    // fill the QMap so every key 0-100 has a value
    for (int i = 0; i <= 100; ++i) {
        if (!(target->contains(i))) {
            QMap<int, QPixmap*>::const_iterator next = target->upperBound(i);
            if (next != target->constEnd()) {
                target->insert(i, next.value());
            }
        }
    }
}

void WBattery::paintEvent(QPaintEvent *) {
    if (!m_pPixmap) {
        hide();
        return;
    }
    QPainter painter(this);
    painter.drawPixmap(0, 0, *m_pPixmap);
}
