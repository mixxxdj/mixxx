/***************************************************************************
                          wnumberbpm.cpp  -  description
                             -------------------
    begin                : Wed Oct 31 2003
    copyright            : (C) 2003 by Tue Haste Andersen
    email                : haste@diku.dk
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtCore>
#include <QtDebug>

#include "wnumberbpm.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"

WNumberBpm::WNumberBpm(const char * group, QWidget * parent) : WNumber(parent)
{
    m_qsText = "BPM: ";
    m_pRateControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate")));
    m_pRateDirControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rate_dir")));
    m_pRateRangeControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "rateRange")));
    m_pBpmControl = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey(group, "bpm")));
    
    connect(m_pRateControl, SIGNAL(valueChanged(double)),
            this, SLOT(setValue(double)));
    connect(m_pRateDirControl, SIGNAL(valueChanged(double)),
            this, SLOT(setValue(double)));
    connect(m_pRateRangeControl, SIGNAL(valueChanged(double)),
            this, SLOT(setValue(double)));
    connect(m_pBpmControl, SIGNAL(valueChanged(double)),
            this, SLOT(setValue(double)));

}


WNumberBpm::~WNumberBpm()
{
    delete m_pRateControl;
    delete m_pRateDirControl;
    delete m_pRateRangeControl;
    delete m_pBpmControl;
}

void WNumberBpm::setValue(double)
{
    WNumber::setValue(m_pBpmControl->get());
}

