#include <QStyleOption>
#include <QStylePainter>

#include "widget/wbattery.h"
#include "util/battery/battery.h"
#include "util/math.h"

WBattery::WBattery(QWidget* parent)
        : WWidget(parent),
          m_pBattery(Battery::getBattery(this)) {
    if (m_pBattery) {
        connect(m_pBattery.data(), SIGNAL(stateChanged()),
                this, SLOT(update()));
    }
}

void WBattery::setup(const QDomNode& node, const SkinContext& context) {
    QDomElement backPath = context.selectElement(node, "BackPath");
    if (!backPath.isNull()) {
        setPixmap(&m_pPixmapBack,
                  context.getPixmapSource(backPath),
                  context.selectScaleMode(backPath, Paintable::TILE),
                  context.getScaleFactor());
    }

    QDomElement unknownPath = context.selectElement(node, "PixmapUnknown");
    if (!unknownPath.isNull()) {
        setPixmap(&m_pPixmapUnknown,
                  context.getPixmapSource(unknownPath),
                  context.selectScaleMode(unknownPath, Paintable::TILE),
                  context.getScaleFactor());
    }

    QDomElement chargedPath = context.selectElement(node, "PixmapCharged");
    if (!chargedPath.isNull()) {
        setPixmap(&m_pPixmapCharged,
                  context.getPixmapSource(chargedPath),
                  context.selectScaleMode(chargedPath, Paintable::TILE),
                  context.getScaleFactor());
    }

    int numberStates = context.selectInt(node, "NumberStates");
    if (numberStates < 0) {
        numberStates = 0;
    }

    m_chargingPixmaps.resize(numberStates);
    m_dischargingPixmaps.resize(numberStates);

    QDomElement pixmapsCharging = context.selectElement(node, "PixmapsCharging");
    if (!pixmapsCharging.isNull()) {
        // TODO(XXX) inline SVG support via context.getPixmapSource.
        QString chargingPath = context.nodeToString(pixmapsCharging);
        Paintable::DrawMode mode = context.selectScaleMode(pixmapsCharging,
                                                           Paintable::TILE);
        for (int i = 0; i < m_chargingPixmaps.size(); ++i) {
            PixmapSource source = context.getPixmapSource(chargingPath.arg(i));
            setPixmap(&m_chargingPixmaps[i], source, mode, context.getScaleFactor());
        }
    }

    QDomElement pixmapsDischarging = context.selectElement(node, "PixmapsDischarging");
    if (!pixmapsDischarging.isNull()) {
        // TODO(XXX) inline SVG support via context.getPixmapSource.
        QString dischargingPath = context.nodeToString(pixmapsDischarging);
        Paintable::DrawMode mode = context.selectScaleMode(pixmapsDischarging,
                                                           Paintable::TILE);
        for (int i = 0; i < m_dischargingPixmaps.size(); ++i) {
            PixmapSource source = context.getPixmapSource(dischargingPath.arg(i));
            setPixmap(&m_dischargingPixmaps[i], source, mode, context.getScaleFactor());
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
    int result = static_cast<int>(dPercentage / 100.0 * numPixmaps);
    result = math_min(numPixmaps - 1, math_max(0, result));
    return result;
}

void WBattery::update() {
    int minutesLeft = m_pBattery ? m_pBattery->getMinutesLeft() : 0;
    Battery::ChargingState chargingState = m_pBattery ?
            m_pBattery->getChargingState() : Battery::UNKNOWN;
    double dPercentage = m_pBattery ? m_pBattery->getPercentage() : 0;

    if (chargingState != Battery::UNKNOWN) {
        setBaseTooltip(QString("%1\%").arg(dPercentage, 0, 'f', 0));
    } else {
        setBaseTooltip(tr("Battery status unknown."));
    }
    m_pCurrentPixmap.clear();
    switch (chargingState) {
        case Battery::CHARGING:
            if (!m_chargingPixmaps.isEmpty()) {
                m_pCurrentPixmap = m_chargingPixmaps[
                    pixmapIndexFromPercentage(dPercentage,
                                              m_chargingPixmaps.size())];
            }
            if (minutesLeft != Battery::TIME_UNKNOWN) {
                appendBaseTooltip("\n" + tr("Time until charged: %1")
                               .arg(formatMinutes(minutesLeft)));
            }
            break;
        case Battery::DISCHARGING:
            if (!m_dischargingPixmaps.isEmpty()) {
                m_pCurrentPixmap = m_dischargingPixmaps[
                    pixmapIndexFromPercentage(dPercentage,
                                              m_dischargingPixmaps.size())];
            }
            if (minutesLeft != Battery::TIME_UNKNOWN) {
                appendBaseTooltip("\n" + tr("Time left: %1")
                               .arg(formatMinutes(minutesLeft)));
            }
            break;
        case Battery::CHARGED:
            m_pCurrentPixmap = m_pPixmapCharged;
            appendBaseTooltip("\n" + tr("Battery fully charged."));
            break;
        case Battery::UNKNOWN:
        default:
            m_pCurrentPixmap = m_pPixmapUnknown;
            break;
    }

    // call parent's update() to show changes, this should call
    // QWidget::update()
    WWidget::update();
}

void WBattery::setPixmap(PaintablePointer* ppPixmap, const PixmapSource& source,
                         Paintable::DrawMode mode, double scaleFactor) {
    PaintablePointer pPixmap = WPixmapStore::getPaintable(source, mode, scaleFactor);
    if (pPixmap.isNull() || pPixmap->isNull()) {
        qDebug() << this << "Error loading pixmap:" << source.getPath();
    } else {
        *ppPixmap = pPixmap;
        if (mode == Paintable::FIXED) {
            setFixedSize(pPixmap->size());
        }
    }
}

void WBattery::paintEvent(QPaintEvent* /*unused*/) {
    QStyleOption option;
    option.initFrom(this);
    QStylePainter p(this);
    p.drawPrimitive(QStyle::PE_Widget, option);

    if (m_pPixmapBack) {
        m_pPixmapBack->draw(rect(), &p);
    }

    if (m_pCurrentPixmap) {
        m_pCurrentPixmap->draw(rect(), &p);
    }
}
