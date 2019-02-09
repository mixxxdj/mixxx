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

WNumberRate::WNumberRate(const char * group, QWidget * parent)
        : WNumber(parent) {
    m_pRateRangeControl = new ControlProxy(group, "rateRange", this);
    m_pRateRangeControl->connectValueChanged(this, &WNumberRate::setValue);
    m_pRateDirControl = new ControlProxy(group, "rate_dir", this);
    m_pRateDirControl->connectValueChanged(this, &WNumberRate::setValue);
    m_pRateControl = new ControlProxy(group, "rate", this);
    m_pRateControl->connectValueChanged(this, &WNumberRate::setValue);
    // Initialize the widget.
    setValue(0);
}

void WNumberRate::setValue(double /*dValue*/) {
    double vsign = m_pRateControl->get() *
            m_pRateRangeControl->get() *
            m_pRateDirControl->get();

    char sign = '+';
    if (vsign < -0.00000001) {
        sign = '-';
    }

    setText(QString(m_skinText).append(sign)
            .append("%1").arg(fabs(vsign) * 100.0, 0, 'f', m_iNoDigits));
}
