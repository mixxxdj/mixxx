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
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "qapplication.h"
#include "midiobject.h"
#include "mathstuff.h"
#include <qdatetime.h>

Rotary::Rotary()
{
    m_pControlObjectRotary = 0;
    m_pControlObjectButton = 0;
    m_dCalibration = 1.;

    m_iFilterLength = kiRotaryFilterMaxLen;

    m_pFilter = new double[m_iFilterLength];
    for (int i=0; i<m_iFilterLength; ++i)
        m_pFilter[i] = 0.;
}

Rotary::~Rotary()
{
    if (running())
    {
        terminate();
        wait();
    }
    delete [] m_pFilter;
}

void Rotary::selectMapping(QString mapping)
{
    if (mapping==kqRotaryMappingP1Phase)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","wheel"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iFilterLength = kiRotaryFilterMaxLen;
    }
    else if (mapping==kqRotaryMappingP2Phase)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","wheel"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iFilterLength = kiRotaryFilterMaxLen;
    }
    else if (mapping==kqRotaryMappingP1Scratch)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel1]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel1]","play"));
        m_iFilterLength = 5;
    }
    else if (mapping==kqRotaryMappingP2Scratch)
    {
        m_pControlObjectRotary = ControlObject::getControl(ConfigKey("[Channel2]","scratch"));
        m_pControlObjectButton = ControlObject::getControl(ConfigKey("[Channel2]","play"));
        m_iFilterLength = 5;
    }
}

void Rotary::run()
{
    while (1)
        getNextEvent();
}

void Rotary::sendRotaryEvent(double dValue)
{
    // Move everything one step backwards in integral buffer
    double dMagnitude = 0.;
    bool bStop = true;
    for (int i=0; i<m_iFilterLength-1; i++)
    {
        m_pFilter[i]=m_pFilter[i+1];
        dMagnitude += m_pFilter[i];
        if (m_pFilter[i]!=0)
            bStop = false;
    }
    m_pFilter[m_iFilterLength-1] = dValue/m_dCalibration;
    dMagnitude += m_pFilter[m_iFilterLength-1];
    dMagnitude /= (double)m_iFilterLength;
/*
    // Range check
    if (m_fMagnitude>63)
        m_fMagnitude = 63;
    else if (m_fMagnitude<-64)
        m_fMagnitude = -64;
*/
//     qDebug("val %f, mag %f", (float)dValue, (float)dMagnitude);

 //   if (m_qCalibrationMutex.tryLock())
    {
        if (m_pControlObjectRotary)
            m_pControlObjectRotary->queueFromThread(dMagnitude);
//        m_qCalibrationMutex.unlock();
    }
/*
    else
    {
        m_dCalibration += dValue;
        m_iCalibrationCount += 1;
    }
*/
}

void Rotary::sendButtonEvent(bool press)
{
    if (m_pControlObjectButton)
    {
        if (press)
            m_pControlObjectButton->queueFromMidi(NOTE_ON, 1);
        else
            m_pControlObjectButton->queueFromMidi(NOTE_OFF, 1);
    }
}

void Rotary::calibrateStart()
{
    // Reset calibration data
    m_dCalibration = 0.;
    m_iCalibrationCount = 0;

    // Start time
    m_pCalibrationTime = new QTime();
    m_pCalibrationTime->start();

    m_qCalibrationMutex.lock();
}

double Rotary::calibrateEnd()
{
    m_qCalibrationMutex.unlock();

    int msec = m_pCalibrationTime->elapsed();

    m_dCalibration /= (double)m_iCalibrationCount; //msec;

    qDebug("Calibration %f, msec %i",m_dCalibration, msec);

    return m_dCalibration;
}

void Rotary::setCalibration(double c)
{
    m_dCalibration = c;
}
