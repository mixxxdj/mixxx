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

#include "wnumberbpm.h"
#include "controlobject.h"

bool WNumberBpm::m_bScaleBpm = true;

WNumberBpm::WNumberBpm(const char *group, QWidget *parent, const char *name) : WNumber(parent, name)
{
    m_qsText = "BPM: ";
    m_pRateControl = ControlObject::getControl(ConfigKey(QString(group), QString("rate")));
    m_pRateDirControl = ControlObject::getControl(ConfigKey(QString(group), QString("rate_dir")));
}


WNumberBpm::~WNumberBpm()
{
}

void WNumberBpm::setValue(double dValue)
{
    if (m_bScaleBpm)
        WNumber::setValue(dValue*(1.+m_pRateControl->getValue()*m_pRateDirControl->getValue()));
    else
        WNumber::setValue(dValue);
}

void WNumberBpm::setScaleBpm(bool bScaleBpm)
{
    m_bScaleBpm = bScaleBpm;
}

