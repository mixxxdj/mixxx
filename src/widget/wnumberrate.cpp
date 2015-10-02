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

#include "controlobject.h"
#include "controlobjectslave.h"
#include "util/math.h"

WNumberRate::WNumberRate(const char * group, QWidget * parent)
        : WNumber(parent) {
    m_pRateRangeControl = new ControlObjectSlave(group, "rateRange", this);
    m_pRateRangeControl->connectValueChanged(SLOT(setValue(double)));
    m_pRateDirControl = new ControlObjectSlave(group, "rate_dir", this);
    m_pRateDirControl->connectValueChanged(SLOT(setValue(double)));
    m_pRateControl = new ControlObjectSlave(group, "rate", this);
    m_pRateControl->connectValueChanged(SLOT(setValue(double)));
    // Initialize the widget.
    setValue(0);
}

WNumberRate::~WNumberRate() {
}

void WNumberRate::setValue(double) {
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
