/***************************************************************************
                          controlpotmeter.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "controlpotmeter.h"
#include "controlengine.h"
#include "controlpushbutton.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new potmeter
   Input:   n - name
            midino - number of the midi controller.
            master - pointer to the control to which the potmeter is
                     attached. This control is acknowledged when the
                     potmeter is changed.
            midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlPotmeter::ControlPotmeter(ConfigKey key, double dMinValue, double dMaxValue) : ControlObject(key)
{
    setRange(dMinValue,dMaxValue);
    setStep(m_dValueRange/10.f);

    ControlPushButton *p;
    p = new ControlPushButton(ConfigKey(key.group, QString(key.item)+"_up"));
    connect(p, SIGNAL(signalUpdateApp(double)), this, SLOT(incValue(double)));
    p = new ControlPushButton(ConfigKey(key.group, QString(key.item)+"_down"));
    connect(p, SIGNAL(signalUpdateApp(double)), this, SLOT(decValue(double)));
}

ControlPotmeter::~ControlPotmeter()
{
}

double ControlPotmeter::getMin()
{
    return m_dMinValue;
}

double ControlPotmeter::getMax()
{
    return m_dMaxValue;
}

void ControlPotmeter::setStep(double dValue)
{
    m_dStep = dValue;
}

void ControlPotmeter::setRange(double dMinValue, double dMaxValue)
{
    m_dMinValue = dMinValue;
    m_dMaxValue = dMaxValue;
    m_dValueRange = m_dMaxValue-m_dMinValue;
    m_dValue = m_dMinValue + 0.5*m_dValueRange;
    //qDebug("%p, min %f, max %f, range %f, val %f",this, m_dMinValue, m_dMaxValue, m_dValueRange, m_dValue);
    updateFromApp();
}

void ControlPotmeter::setValueFromWidget(double dValue)
{
    m_dValue = m_dMinValue + (dValue/127.)*m_dValueRange;

    updateFromWidget();
}

void ControlPotmeter::setValueFromApp(double dValue)
{
    if (dValue>m_dMaxValue)
        m_dValue = m_dMaxValue;
    else if (dValue<m_dMinValue)
        m_dValue = m_dMinValue;
    else
        m_dValue = dValue;

    updateFromApp();
}

void ControlPotmeter::setValueFromEngine(double dValue)
{
    if (dValue>m_dMaxValue)
    {
        m_dValue = m_dMaxValue;
        updateAll();
    }
    else if (dValue<m_dMinValue)
    {
        m_dValue = m_dMinValue;
        updateAll();
    }
    else
    {
        m_dValue = dValue;
        updateFromEngine();
    }
}

void ControlPotmeter::setValueFromMidi(MidiCategory, int v)
{
    m_dValue = m_dMinValue + ((double)v/127.)*m_dValueRange;
    updateFromMidi();
}

void ControlPotmeter::updateWidget()
{
    emit(signalUpdateWidget(127.*(m_dValue-m_dMinValue)/m_dValueRange));
}

void ControlPotmeter::incValue(double keypos)
{
    if (keypos>0)
    {
        if (m_dValue+m_dStep>m_dMaxValue)
            m_dValue = m_dMaxValue;
        else
            m_dValue += m_dStep;

        updateAll();
    }
}

void ControlPotmeter::decValue(double keypos)
{
    if (keypos>0)
    {
        if (m_dValue-m_dStep<m_dMinValue)
            m_dValue = m_dMinValue;
        else
            m_dValue -= m_dStep;

        updateAll();
    }
}


