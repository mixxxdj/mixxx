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

WNumberRate::WNumberRate(const char *group, QWidget *parent, const char *name) : WNumber(parent, name)
{
    m_pRateControl = ControlObject::getControl(ConfigKey(QString(group), QString("rate")));
    m_pRateDirControl = ControlObject::getControl(ConfigKey(QString(group), QString("rate_dir")));
}

WNumberRate::~WNumberRate()
{
}

void WNumberRate::setValue(double)
{
    double vsign = m_pRateControl->getValue()*m_pRateDirControl->getValue();
    double v = fabs(vsign);
    int v1=0,v2=0,v3=0;

    v1 = (int)floor((v-floor(v))*10.);
    v2 = (int)(floor((v-floor(v))*100.))%10;
    v3 = (int)(floor((v-floor(v))*1000.))%10;

    QString sign = "+";
    if (vsign<0.)
        sign = "-";

    m_pLabel->setText(QString(m_qsText).append("%1%2%3.%4%").arg(sign).arg(v1,1,10).arg(v2,1,10).arg(v3,1,10));
}

