#include <QStyleOption>
#include <QStylePainter>

#include "widget/wbattery.h"
#include "util/battery/battery.h"
#include "defs.h"

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
    if (context.hasNode(node, "BackPath")) {
        setPixmap(&m_pPixmapBack, context.selectString(node, "BackPath"));
    }

    if (context.hasNode(node, "PixmapUnknown")) {
        setPixmap(&m_pPixmapUnknown, context.selectString(node, "PixmapUnknown"));
    }

    if (context.hasNode(node, "PixmapCharged")) {
        setPixmap(&m_pPixmapCharged, context.selectString(node, "PixmapCharged"));
    }

    int numberStates = context.selectInt(node, "NumberStates");
    if (numberStates < 0) {
        numberStates = 0;
    }

    m_chargingPixmaps.resize(numberStates);
    m_dischargingPixmaps.resize(numberStates);

    if (context.hasNode(node, "PixmapsCharging")) {
        QString chargingPath = context.selectString(node, "PixmapsCharging");
        for (int i = 0; i < m_chargingPixmaps.size(); ++i) {
            setPixmaps(&m_chargingPixmaps, i,
                       context.getSkinPath(chargingPath.arg(i)));
        }
    }

    if (context.hasNode(node, "PixmapsDischarging")) {
        QString dischargingPath = context.selectString(node, "PixmapsDischarging");
        for (int i = 0; i < m_dischargingPixmaps.size(); ++i) {
            setPixmaps(&m_dischargingPixmaps, i,
                       context.getSkinPath(dischargingPath.arg(i)));
        }
    }

    if (m_pBattery) {
        m_pBattery->update();
    }
}

QString formatMinutes(int minutes) {
    if (minutes < 60) {
        return QObject::tr("%1 minutes").arg(minutes);
    }
    return QObject::tr("%1:%2").arg(minutes/60)
            .arg(minutes%60, 2, 10, QChar('0'));
}

int pixmapIndexFromPercentage(double dPercentage, int numPixmaps) {
    // See WDisplay::getActivePixmapIndex for more info on this.
    int result = static_cast<int>(dPercentage * numPixmaps - 0.00001);
    result = math_min(numPixmaps - 1, math_max(0, result));
    return result;
}

void WBattery::update() {
    int minutesLeft = m_pBattery ? m_pBattery->getMinutesLeft() : 0;
    Battery::ChargingState csChargingState = m_pBattery ?
            m_pBattery->getChargingState() : Battery::UNKNOWN;
    double dPercentage = m_pBattery ? m_pBattery->getPercentage() : 0;

    m_pCurrentPixmap.clear();
    switch (csChargingState) {
        case Battery::CHARGING:
            if (!m_chargingPixmaps.isEmpty()) {
                m_pCurrentPixmap = m_chargingPixmaps[
                    pixmapIndexFromPercentage(dPercentage,
                                              m_chargingPixmaps.size())];
            }
            if (minutesLeft == -1) {
                setBaseTooltip(tr("Time until charged unknown."));
            } else {
                setBaseTooltip(tr("Time until charged: %1")
                               .arg(formatMinutes(minutesLeft)));
            }
            break;
        case Battery::DISCHARGING:
            if (!m_dischargingPixmaps.isEmpty()) {
                m_pCurrentPixmap = m_dischargingPixmaps[
                    pixmapIndexFromPercentage(dPercentage,
                                              m_dischargingPixmaps.size())];
            }
            if (minutesLeft == -1) {
                setBaseTooltip(tr("Time left unknown."));
            } else {
                setBaseTooltip(tr("Time left: %1")
                               .arg(formatMinutes(minutesLeft)));
            }
            break;
        case Battery::CHARGED:
            m_pCurrentPixmap = m_pPixmapCharged;
            setBaseTooltip(tr("Battery fully charged."));
            break;
        case Battery::UNKNOWN:
        default:
            m_pCurrentPixmap = m_pPixmapUnknown;
            setBaseTooltip(tr("Battery status unknown."));
            break;
    }

    // call parent's update() to show changes, this should call
    // QWidget::update()
    WWidget::update();
}

void WBattery::setPixmap(PaintablePointer* ppPixmap, const QString& filename) {
    PaintablePointer pPixmap = WPixmapStore::getPaintable(filename);

    if (pPixmap.isNull() || pPixmap->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading pixmap:" << filename;
    } else {
        *ppPixmap = pPixmap;
        setFixedSize(pPixmap->size());
    }
}

void WBattery::setPixmaps(QVector<PaintablePointer>* pPixmaps,
                          int iPos, const QString& filename) {
    if (iPos < 0 || iPos >= pPixmaps->size()) {
        return;
    }

    PaintablePointer pPixmap = WPixmapStore::getPaintable(filename);

    if (pPixmap.isNull() || pPixmap->isNull()) {
        qDebug() << metaObject()->className()
                 << "Error loading pixmap:" << filename;
    } else {
        (*pPixmaps)[iPos] = pPixmap;
        setFixedSize(pPixmap->size());
    }
}

void WBattery::paintEvent(QPaintEvent*) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(0, 0, &p);
    }

    if (m_pCurrentPixmap) {
        m_pCurrentPixmap->draw(0, 0, &p);
    }
}
