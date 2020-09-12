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
    m_pRateRangeControl->connectValueChanged(SLOT(setValue(double)));
    m_pRateDirControl = new ControlProxy(group, "rate_dir", this);
    m_pRateDirControl->connectValueChanged(SLOT(setValue(double)));
    m_pRateControl = new ControlProxy(group, "rate", this);
    m_pRateControl->connectValueChanged(SLOT(setValue(double)));
    // Initialize the widget.
    setValue(0);
}

void WNumberRate::setValue(double dValue) {
    double digitFactor = pow(10, m_iNoDigits);
    // Calculate percentage rounded to the number of digits specified by iNoDigits
    double percentage = round((dValue - 1) * 100.0 * digitFactor) / digitFactor;

    QString sign(' ');
    if (percentage > 0) {
        sign = '+';
    }
    if (percentage < 0) {
        sign = '-';
    }

    setText(m_skinText + sign + QString::number(fabs(percentage), 'f', m_iNoDigits));
}
