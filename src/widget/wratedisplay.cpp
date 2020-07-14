#include <QDebug>

#include "widget/wratedisplay.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"

WRateDisplay::WRateDisplay(const QString& group, QWidget* parent)
        : WNumber(parent) {
    m_pRateRangeControl = new ControlProxy(group, "rateRange", this, ControlFlag::NoAssertIfMissing);
    m_pRateRangeControl->connectValueChanged(this, &WRateDisplay::setValue);
    m_pRateDirControl = new ControlProxy(group, "rate_dir", this, ControlFlag::NoAssertIfMissing);
    m_pRateDirControl->connectValueChanged(this, &WRateDisplay::setValue);
}

void WRateDisplay::setup(const QDomNode& node, const SkinContext& context) {
    WNumber::setup(node, context);

    QDomElement rateDisplayPosition = context.selectElement(node, "Position");
    m_nodePosition = rateDisplayPosition.text() == "Top" ? VerticalPosition::Top : VerticalPosition::Bottom;

    // Initialize the widget (overrides the base class initial value).
    setValue();
}

void WRateDisplay::setValue() {
    double range = m_pRateRangeControl->get();
    double direction = m_pRateDirControl->get();

    if (nodePosition == VerticalPosition::Top) {
        qDebug() << "Position: TOP";
    }
    if(nodePosition == VerticalPosition::Bottom) {
        qDebug() << "Position: BOTTOM";
    }
    if(direction > 0) {
        qDebug() << "Direction > 0";
    }
    if(direction < 0) {
        qDebug() << "Direction < 0";
    }

    QString sign('-');
    if(nodePosition == VerticalPosition::Top && direction > 0) {
        sign = '+';
    }

    if(nodePosition == VerticalPosition::Bottom) {
        sign = '-';
        if(direction < 0) {
            sign = '+';
        }
    }

    setText(sign
        .append("%1").arg(range * 100)
        .append("\%")
    );
}
