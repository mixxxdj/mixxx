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
#include "wnumberrate.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

WNumberRate::WNumberRate(const char *group, QWidget *parent, const char *name) : WNumber(parent, name)
{
    m_pRateRangeControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rateRange")));
    m_pRateDirControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate_dir")));
    m_pRateControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate")));
}

WNumberRate::~WNumberRate()
{
    delete m_pRateControl;
    delete m_pRateDirControl;
    delete m_pRateRangeControl;
}

void WNumberRate::setValue(double)
{    
    double vsign = m_pRateControl->get()*m_pRateRangeControl->get()*m_pRateDirControl->get();

	m_pLabel->setText(QString(m_qsText).append("%1").arg(vsign*100., 0, 'f', 2));
}

