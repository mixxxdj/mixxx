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
#include "controlobjectthreadmain.h"

WNumberPos::WNumberPos(const char * group, QWidget * parent) : WNumber(parent)
{
    m_dDuration = 0.;
    m_dOldValue = 0.;
    m_qsText = "";
    m_bRemain = false;

    m_pShowDurationRemaining = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Controls]", "ShowDurationRemaining")));
    connect(m_pShowDurationRemaining, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRemain(double)));
    slotSetRemain(m_pShowDurationRemaining->get());

    m_pRateControl = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey(group, "rate")));
    m_pRateDirControl = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey(group, "rate_dir")));
    m_pDurationControl = new ControlObjectThreadWidget(ControlObject::getControl(ConfigKey(group, "duration")));
    connect(m_pDurationControl, SIGNAL(valueChanged(double)), this, SLOT(slotSetDuration(double)));

    // Tell the duration CO to signal us the update because we might be created
    // after it is set on load.
    m_pDurationControl->emitValueChanged();

}

WNumberPos::~WNumberPos()
{
}

void WNumberPos::slotSetDuration(double dDuration)
{
    m_dDuration = dDuration;
    setValue(m_dOldValue);
}

void WNumberPos::setValue(double dValue)
{
    m_dOldValue = dValue;

    double v  = dValue*(m_dDuration/127.);
    double v2 = m_dDuration;

    if (m_bRemain)
        v = m_dDuration-v;

    int min1=0,min2=0,sec1=0,sec2=0,msec1=0,msec2=0;
    int minv21=0,minv22=0,secv21=0,secv22=0,msecv21=0,msecv22=0;
    if (v>0.)
    {
        min1 = (int)(floor(v/600.))%100;
        min2 = (int)(floor(v/60.))%10;
        sec1 = (int)(floor(v/10.))%6;
        sec2 = (int)(floor(v))%10;
        msec1 = (int)floor((v-floor(v))*10.);
        msec2 = (int)(floor((v-floor(v))*100.))%10;
    }

    if (v2>0.)
    {
        minv21 = (int)(floor(v2/600.))%100;
        minv22 = (int)(floor(v2/60.))%10;
        secv21 = (int)(floor(v2/10.))%6;
        secv22 = (int)(floor(v2))%10;
        msecv21 = (int)floor((v2-floor(v2))*10.);
        msecv22 = (int)(floor((v2-floor(v2))*100.))%10;
    }

/*
    if (v<30. && v>0.)
        m_pLabel->setPaletteForegroundColor(QColor(255,0,0));
    else
        m_pLabel->setPaletteForegroundColor(m_qFgColor);
 */

    m_pLabel->setText(QString(m_qsText).append("%1%2:%3%4.%5%6 / %7%8:%9%10.%11%12")
                      .arg(min1,1,10).arg(min2,1,10).arg(sec1,1,10).arg(sec2,1,10).arg(msec1,1,10).arg(msec2,1,10)
                      .arg(minv21,1,10).arg(minv22,1,10).arg(secv21,1,10).arg(secv22,1,10).arg(msecv21,1,10).arg(msecv22,1,10));
}

void WNumberPos::slotSetRemain(double remain) {
    setRemain(remain == 1.0f);
}

void WNumberPos::setRemain(bool bRemain)
{
    m_bRemain = bRemain;

    // Shift display state between showing position and remaining
    /*
    if (m_bRemain)
        m_qsText = "R: ";
    else
        m_qsText = "E: ";
    */

    // Have the widget redraw itself with its current value.
    setValue(m_dOldValue);
}






