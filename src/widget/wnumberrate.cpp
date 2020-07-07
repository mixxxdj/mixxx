//
// C++ Implementation: wnumberrate
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "widget/wnumberrate.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"

WNumberRate::WNumberRate(const QString& group, QWidget* parent)
        : WNumber(parent) {
    m_pRateRatio = new ControlProxy(group, "rate_ratio", this, ControlFlag::NoAssertIfMissing);
    m_pRateRatio->connectValueChanged(this, &WNumberRate::setValue);
}

void WNumberRate::setup(const QDomNode& node, const SkinContext& context) {
    WNumber::setup(node, context);

    // Initialize the widget (overrides the base class initial value.
    setValue(m_pRateRatio->get());
}

void WNumberRate::setValue(double dValue) {
    double vsign = dValue - 1;

    char sign = '+';
    if (vsign < -0.00000001) {
        sign = '-';
    }

    setText(QString(m_skinText).append(sign)
            .append("%1").arg(fabs(vsign) * 100.0, 0, 'f', m_iNoDigits));
}
