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

WNumberPos::WNumberPos(QWidget *parent, const char *name) : WNumber(parent, name)
{
    m_iDuration = 0;
    m_qsText = "Pos: ";
    m_bRemain = false;
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
        v = (float)m_iDuration-v;

    int min1 = v/600.;
    int min2 = (int)(v/60.)%10;
    int sec1 = ((int)v%60)/10;
    int sec2 = (int)v%10;
    int msec1 = (int)((v-floor(v))*10.);
    int msec2 = (int)((v-floor(v))*100.)%10;

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


