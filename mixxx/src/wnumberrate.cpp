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
    
    double v = fabs(vsign);
    int v1=0,v2=0,v3=0,v4=0;

    v1 = (int)floor((v-floor(v))*10.);
    v2 = (int)(floor((v-floor(v))*100.))%10;
    v3 = (int)(floor((v-floor(v))*1000.))%10;
    v4 = (int)(floor((v-floor(v))*10000.))%10;

    QString sign = "+";
    if (vsign<0.)
        sign = "-";

    m_pLabel->setText(QString(m_qsText).append("%1%2%3.%4%5").arg(sign).arg(v1,1,10).arg(v2,1,10).arg(v3,1,10).arg(v4,1,10));
}

