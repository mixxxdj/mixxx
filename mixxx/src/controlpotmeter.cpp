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

#include "controlpushbutton.h"
#include "controlpotmeter.h"

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
    setSmallStep(m_dValueRange/100.f);

    // ControlPushButton * p;
    m_pControlUp = new ControlPushButton(ConfigKey(key.group, QString(key.item)+"_up"));
    connect(m_pControlUp, SIGNAL(valueChanged(double)), this, SLOT(incValue(double)));
    m_pControlDown = new ControlPushButton(ConfigKey(key.group, QString(key.item)+"_down"));
    connect(m_pControlDown, SIGNAL(valueChanged(double)), this, SLOT(decValue(double)));
    m_pControlUpSmall = new ControlPushButton(ConfigKey(key.group, QString(key.item)+"_up_small"));
    connect(m_pControlUpSmall, SIGNAL(valueChanged(double)), this, SLOT(incSmallValue(double)));
    m_pControlDownSmall = new ControlPushButton(ConfigKey(key.group, QString(key.item)+"_down_small"));
    connect(m_pControlDownSmall, SIGNAL(valueChanged(double)), this, SLOT(decSmallValue(double)));
}

ControlPotmeter::~ControlPotmeter()
{
	// If the leaked controles are deleted by mixxx.cpp
	// The subControls of ControlPotentiometer may have bin already delete.
	// So check it before
	/*
	delete m_pControlUp;
    delete m_pControlDown;
    delete m_pControlUpSmall;
    delete m_pControlDownSmall;
	*/

	ControlObject* p;

	p = ControlObject::getControl(ConfigKey(getKey().group, QString(getKey().item)+"_up"));
	if (p) {
		delete p;
	}
	p = ControlObject::getControl(ConfigKey(getKey().group, QString(getKey().item)+"_down"));
	if (p) {
		delete p;
	}
	p = ControlObject::getControl(ConfigKey(getKey().group, QString(getKey().item)+"_up_small"));
	if (p) {
		delete p;
	}
	p = ControlObject::getControl(ConfigKey(getKey().group, QString(getKey().item)+"_down_small"));
	if (p) {
		delete p;
	}
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

void ControlPotmeter::setSmallStep(double dValue)
{
    m_dSmallStep = dValue;
}

void ControlPotmeter::setRange(double dMinValue, double dMaxValue)
{
    m_dMinValue = dMinValue;
    m_dMaxValue = dMaxValue;
    m_dValueRange = m_dMaxValue-m_dMinValue;
    m_dValue = m_dMinValue + 0.5*m_dValueRange;
    //qDebug() << "" << this << ", min " << m_dMinValue << ", max " << m_dMaxValue << ", range " << m_dValueRange << ", val " << m_dValue;
}

double ControlPotmeter::getValueToWidget(double dValue)
{
    double out = (dValue-m_dMinValue)/m_dValueRange;
    return (out < 0.5) ? out*128. : out*126. + 1.;
}

double ControlPotmeter::GetMidiValue()
{
    double out = (m_dValue-m_dMinValue)/m_dValueRange;
    return (out < 0.5) ? out*128. : out*126. + 1.;
}

double ControlPotmeter::getValueFromWidget(double dValue)
{
    double out = (dValue < 64) ? dValue / 128. : (dValue-1) / 126.;
    return m_dMinValue + out * m_dValueRange;
}

void ControlPotmeter::setValueFromThread(double dValue)
{
    if (dValue == m_dValue) return;

    if (dValue>m_dMaxValue)
        m_dValue = m_dMaxValue;
    else if (dValue<m_dMinValue)
        m_dValue = m_dMinValue;
    else
        m_dValue = dValue;
    emit(valueChanged(m_dValue));
}

void ControlPotmeter::setValueFromEngine(double dValue)
{
    if (dValue>m_dMaxValue)
        m_dValue = m_dMaxValue;
    else if (dValue<m_dMinValue)
        m_dValue = m_dMinValue;
    else
        m_dValue = dValue;
    emit(valueChangedFromEngine(m_dValue));
}

void ControlPotmeter::setValueFromMidi(MidiCategory, double v)
{
    double out = (v < 64) ? v / 128. : (v-1) / 126.;
    m_dValue = m_dMinValue + out*m_dValueRange;
    emit(valueChanged(m_dValue));
}

void ControlPotmeter::incValue(double keypos)
{
    if (keypos>0)
    {
        m_dValue += m_dStep;
        if (m_dValue > m_dMaxValue)
            m_dValue = m_dMaxValue;
        emit(valueChanged(m_dValue));

        // incValue will be activated by assosiated _up or _down ControlObject, and thus it is safe to update all proxies.
        updateProxies(0);
    }
}

void ControlPotmeter::decValue(double keypos)
{
    if (keypos>0)
    {
        m_dValue -= m_dStep;
        if (m_dValue < m_dMinValue)
            m_dValue = m_dMinValue;
        emit(valueChanged(m_dValue));

        // decValue will be activated by assosiated _up or _down ControlObject, and thus it is safe to update all proxies.
        updateProxies(0);
    }
}

void ControlPotmeter::incSmallValue(double keypos)
{
    if (keypos>0)
    {
        m_dValue += m_dSmallStep;
        if (m_dValue > m_dMaxValue)
            m_dValue = m_dMaxValue;
        emit(valueChanged(m_dValue));

        // incSmallValue will be activated by assosiated _up_small or _down_small ControlObject, and thus it is safe to update all proxies.
        updateProxies(0);
    }
}

void ControlPotmeter::decSmallValue(double keypos)
{
    if (keypos>0)
    {
        m_dValue -= m_dSmallStep;
        if (m_dValue < m_dMinValue)
            m_dValue = m_dMinValue;
        emit(valueChanged(m_dValue));

        // decSmallValue will be activated by assosiated _up_small or _down_small ControlObject, and thus it is safe to update all proxies.
        updateProxies(0);
    }
}
