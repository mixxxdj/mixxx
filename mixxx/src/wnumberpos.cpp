//
// C++ Implementation: wnumberpos
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wnumberpos.h"
#include <math.h>
#include "controlobject.h"
#include "controlobjectthreadwidget.h"

WNumberPos::WNumberPos(const char *group, QWidget *parent, const char *name) : WNumber(parent, name)
{
    m_iDuration = 0;
    m_qsText = "Pos: ";
    m_bRemain = false;
    m_pRateControl = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey(group, "rate")));
    m_pRateDirControl = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey(group, "rate_dir")));
}

WNumberPos::~WNumberPos()
{
}

void WNumberPos::setDuration(int iDuration)
{
    m_iDuration = iDuration;
}

void WNumberPos::setValue(double dValue)
{
    double v = dValue*((float)m_iDuration/127.);
    if (m_bRemain)
        v = ((float)m_iDuration-v)*(1.-m_pRateControl->get()*m_pRateDirControl->get());

    int min1=0,min2=0,sec1=0,sec2=0,msec1=0,msec2=0;
    if (v>0.)
    {
        min1 = (int)(floor(v/600.))%10;
        min2 = (int)(floor(v/60.))%10;
        sec1 = (int)(floor(v/10.))%6;
        sec2 = (int)(floor(v))%10;
        msec1 = (int)floor((v-floor(v))*10.);
        msec2 = (int)(floor((v-floor(v))*100.))%10;
    }

/*
    if (v<30. && v>0.)
        m_pLabel->setPaletteForegroundColor(QColor(255,0,0));
    else
        m_pLabel->setPaletteForegroundColor(m_qFgColor);
*/

    m_pLabel->setText(QString(m_qsText).append("%1%2:%3%4:%5%6").arg(min1,1,10).arg(min2,1,10).arg(sec1,1,10).arg(sec2,1,10).arg(msec1,1,10).arg(msec2,1,10));
}

void WNumberPos::setRemain(bool bRemain)
{
    m_bRemain = bRemain;

    // Shift display state between showing position and remaining
    if (m_bRemain)
        m_qsText = "Rem: ";
    else
        m_qsText = "Pos: ";
}




