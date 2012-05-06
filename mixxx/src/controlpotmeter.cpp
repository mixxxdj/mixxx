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

    // These controls are deleted when this ControlPotmeter is since we set
    // their parent as this.
    ControlPushButton* controlUp = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_up"));
    controlUp->setParent(this);
    connect(controlUp, SIGNAL(valueChanged(double)),
            this, SLOT(incValue(double)));

    ControlPushButton* controlDown = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_down"));
    controlDown->setParent(this);
    connect(controlDown, SIGNAL(valueChanged(double)),
            this, SLOT(decValue(double)));

    ControlPushButton* controlUpSmall = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_up_small"));
    controlUpSmall->setParent(this);
    connect(controlUpSmall, SIGNAL(valueChanged(double)),
            this, SLOT(incSmallValue(double)));

    ControlPushButton* controlDownSmall = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_down_small"));
    controlDownSmall->setParent(this);
    connect(controlDownSmall, SIGNAL(valueChanged(double)),
            this, SLOT(decSmallValue(double)));

    ControlPushButton* controlDefault = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_default"));
    controlDefault->setParent(this);
    connect(controlDefault, SIGNAL(valueChanged(double)),
            this, SLOT(setToDefault(double)));

    ControlPushButton* controlZero = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_zero"));
    controlZero->setParent(this);
    connect(controlZero, SIGNAL(valueChanged(double)),
            this, SLOT(setToZero(double)));

    ControlPushButton* controlOne = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_one"));
    controlOne->setParent(this);
    connect(controlOne, SIGNAL(valueChanged(double)),
            this, SLOT(setToOne(double)));

    ControlPushButton* controlMinusOne = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_set_minus_one"));
    controlMinusOne->setParent(this);
    connect(controlMinusOne, SIGNAL(valueChanged(double)),
            this, SLOT(setToMinusOne(double)));

    ControlPushButton* controlToggle = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_toggle"));
    controlToggle->setParent(this);
    connect(controlToggle, SIGNAL(valueChanged(double)),
            this, SLOT(toggleValue(double)));

    ControlPushButton* controlMinusToggle = new ControlPushButton(
        ConfigKey(key.group, QString(key.item) + "_minus_toggle"));
    controlMinusToggle->setParent(this);
    connect(controlMinusToggle, SIGNAL(valueChanged(double)),
            this, SLOT(toggleMinusValue(double)));
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
    m_dDefaultValue = m_dValue;
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

void ControlPotmeter::setValueFromMidi(MidiOpCode o, double v)
{
    Q_UNUSED(o);
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

void ControlPotmeter::setToZero(double keypos)
{
    if (keypos>0)
    {
        m_dValue = 0.0;
        emit(valueChanged(m_dValue));
        updateProxies(0);
    }
}

void ControlPotmeter::setToOne(double keypos)
{
    if (keypos>0)
    {
        m_dValue = 1.0;
        emit(valueChanged(m_dValue));
        updateProxies(0);
    }
}

void ControlPotmeter::setToMinusOne(double keypos)
{
    if (keypos>0)
    {
        m_dValue = -1.0;
        emit(valueChanged(m_dValue));
        updateProxies(0);
    }
}

void ControlPotmeter::setToDefault(double v) {
    if (v > 0) {
        m_dValue = m_dDefaultValue;
        emit(valueChanged(m_dValue));
        updateProxies(0);
    }
}

void ControlPotmeter::toggleValue(double keypos)
{
    if (keypos>0)
    {
        if (m_dValue > 0.0)
        {
            m_dValue = 0.0;
        }
        else
        {
            m_dValue = 1.0;
        }
        emit(valueChanged(m_dValue));
        updateProxies(0);
    }
}

void ControlPotmeter::toggleMinusValue(double keypos)
{
    if (keypos>0)
    {
        if (m_dValue > 0.0)
        {
            m_dValue = -1.0;
        }
        else
        {
            m_dValue = 1.0;
        }
        emit(valueChanged(m_dValue));
        updateProxies(0);
    }
}

