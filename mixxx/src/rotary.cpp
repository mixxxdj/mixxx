/***************************************************************************
                          rotary.cpp  -  description
                             -------------------
    begin                : Tue Sep 21 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#include "rotary.h"
#include "mathstuff.h"

Rotary::Rotary()
{
    m_dCalibration = 1.;
    m_dLastValue = 0.;

    m_iFilterLength = kiRotaryFilterMaxLen;
    m_iFilterPos = 0;
    m_pFilter = new double[m_iFilterLength];
    for (int i=0; i<m_iFilterLength; ++i)
        m_pFilter[i] = 0.;
}

Rotary::~Rotary()
{
    delete [] m_pFilter;
}

/*
void Rotary::selectMapping(QString mapping)
{
    if (mapping==kqRotaryMappingP1Phase)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","wheel"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iFilterLength = kiRotaryFilterMaxLen;
        m_iFilterPos = 0;
    }
    else if (mapping==kqRotaryMappingP2Phase)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","wheel"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iFilterLength = kiRotaryFilterMaxLen;
        m_iFilterPos = 0;
    }
    else if (mapping==kqRotaryMappingP1Scratch)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iFilterLength = 5;
        m_iFilterPos = 0;
    }
    else if (mapping==kqRotaryMappingP2Scratch)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iFilterLength = 5;
        m_iFilterPos = 0;
    }
    else if (mapping==kqRotaryMappingP1Search)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","rateSearch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iFilterLength = kiRotaryFilterMaxLen;
        m_iFilterPos = 0;
    }
    else if (mapping==kqRotaryMappingP2Search)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","realsearch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iFilterLength = kiRotaryFilterMaxLen;
        m_iFilterPos = 0;
    }
}
*/

double Rotary::filter(double dValue)
{
    // Update filter buffer
    m_pFilter[m_iFilterPos] = dValue/m_dCalibration;
    m_iFilterPos = (m_iFilterPos+1)%m_iFilterLength;

    double dMagnitude = 0.;
    for (int i=0; i<m_iFilterLength; i++)
    {
        dMagnitude += m_pFilter[i];
    }
    dMagnitude /= (double)m_iFilterLength;
//     qDebug("filter in %f, out %f",dValue,dMagnitude);
    
    if (m_dLastValue!=dMagnitude)
        return dMagnitude;
    
    m_dLastValue = dMagnitude;
    
    return dMagnitude;
}

void Rotary::calibrate(double dValue)
{
    m_dCalibration += dValue;
    m_iCalibrationCount += 1;
}

void Rotary::calibrateStart()
{
    // Reset calibration data
    m_dCalibration = 0.;
    m_iCalibrationCount = 0;
}

double Rotary::calibrateEnd()
{
    m_dCalibration /= (double)m_iCalibrationCount;

    qDebug("Calibration %f, count %i",m_dCalibration,m_iCalibrationCount);

    return m_dCalibration;
}

void Rotary::setCalibration(double c)
{
    m_dCalibration = c;
}

void Rotary::setFilterLength(int i)
{
    if (i>kiRotaryFilterMaxLen)
        m_iFilterLength = kiRotaryFilterMaxLen;
    else if (i<1)
        m_iFilterLength = 1;
    else
        m_iFilterLength = i;
}
