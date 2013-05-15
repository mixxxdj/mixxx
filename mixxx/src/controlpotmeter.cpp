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
ControlPotmeter::ControlPotmeter(ConfigKey key, double dMinValue, double dMaxValue)
    : ControlObject(key) {
    setRange(dMinValue, dMaxValue);
    setStep(m_dValueRange / 10.f);
    setSmallStep(m_dValueRange / 100.f);

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

double ControlPotmeter::getMin() const {
    return m_dMinValue;
}

double ControlPotmeter::getMax() const {
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
    m_dValueRange = m_dMaxValue - m_dMinValue;
    double default_value = m_dMinValue + 0.5 * m_dValueRange;

    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlPotmeterBehavior(dMinValue, dMaxValue));
        delete pOldBehavior;
    }

    setDefaultValue(default_value);
    set(default_value);
    //qDebug() << "" << this << ", min " << m_dMinValue << ", max " << m_dMaxValue << ", range " << m_dValueRange << ", val " << m_dValue;
}

void ControlPotmeter::incValue(double keypos)
{
    if (keypos>0)
    {
        double value = get();
        value += m_dStep;
        if (value > m_dMaxValue) {
            value = m_dMaxValue;
        }
        set(value);
    }
}

void ControlPotmeter::decValue(double keypos)
{
    if (keypos>0)
    {
        double value = get();

        value -= m_dStep;
        if (value < m_dMinValue) {
            value = m_dMinValue;
        }
        set(value);
    }
}

void ControlPotmeter::incSmallValue(double keypos)
{
    if (keypos>0)
    {
        double value = get();
        value += m_dSmallStep;
        if (value > m_dMaxValue) {
            value = m_dMaxValue;
        }
        set(value);
    }
}

void ControlPotmeter::decSmallValue(double keypos)
{
    if (keypos>0)
    {
        double value = get();
        value -= m_dSmallStep;
        if (value < m_dMinValue) {
            value = m_dMinValue;
        }
        set(value);
    }
}

void ControlPotmeter::setToZero(double keypos)
{
    if (keypos>0)
    {
        set(0.0);
    }
}

void ControlPotmeter::setToOne(double keypos)
{
    if (keypos>0)
    {
        set(1.0);
    }
}

void ControlPotmeter::setToMinusOne(double keypos)
{
    if (keypos>0)
    {
        set(-1.0);
    }
}

void ControlPotmeter::setToDefault(double v) {
    if (v > 0) {
        reset();
    }
}

void ControlPotmeter::toggleValue(double keypos) {
    if (keypos>0) {
        double value = get();
        if (value > 0.0) {
            set(0.0);
        } else {
            set(1.0);
        }
    }
}

void ControlPotmeter::toggleMinusValue(double keypos) {
    if (keypos>0) {
        double value = get();
        if (value > 0.0) {
            set(-1.0);
        } else {
            set(1.0);
        }
    }
}

