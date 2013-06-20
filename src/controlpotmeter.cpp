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
#include "controlobjectthread.h"

ControlPotmeter::ControlPotmeter(ConfigKey key, double dMinValue, double dMaxValue)
        : ControlObject(key),
          m_controls(key) {
    setRange(dMinValue, dMaxValue);
    setStep(m_dValueRange / 10.f);
    setSmallStep(m_dValueRange / 100.f);
}

ControlPotmeter::~ControlPotmeter() {
}

double ControlPotmeter::getMin() const {
    return m_dMinValue;
}

double ControlPotmeter::getMax() const {
    return m_dMaxValue;
}

void ControlPotmeter::setStep(double dValue) {
    m_controls.setStep(dValue);
}

void ControlPotmeter::setSmallStep(double dValue) {
    m_controls.setSmallStep(dValue);
}

void ControlPotmeter::setRange(double dMinValue, double dMaxValue) {
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

PotmeterControls::PotmeterControls(ConfigKey key)
        : m_pControl(new ControlObjectThread(key)),
          m_dStep(0),
          m_dSmallStep(0) {
    // These controls are deleted when the ControlPotmeter is since
    // PotmeterControls is a member variable of the associated ControlPotmeter
    // and the push-button controls are parented to the PotmeterControls.

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

PotmeterControls::~PotmeterControls() {
    delete m_pControl;
}

void PotmeterControls::incValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        value += m_dStep;
        m_pControl->set(value);
    }
}

void PotmeterControls::decValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        value -= m_dStep;
        m_pControl->set(value);
    }
}

void PotmeterControls::incSmallValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        value += m_dSmallStep;
        m_pControl->set(value);
    }
}

void PotmeterControls::decSmallValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        value -= m_dSmallStep;
        m_pControl->set(value);
    }
}

void PotmeterControls::setToZero(double v) {
    if (v > 0) {
        m_pControl->set(0.0);
    }
}

void PotmeterControls::setToOne(double v) {
    if (v > 0) {
        m_pControl->set(1.0);
    }
}

void PotmeterControls::setToMinusOne(double v) {
    if (v > 0) {
        m_pControl->set(-1.0);
    }
}

void PotmeterControls::setToDefault(double v) {
    if (v > 0) {
        m_pControl->reset();
    }
}

void PotmeterControls::toggleValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        m_pControl->set(value > 0.0 ? 0.0 : 1.0);
    }
}

void PotmeterControls::toggleMinusValue(double v) {
    if (v > 0) {
        double value = m_pControl->get();
        m_pControl->set(value > 0.0 ? -1.0 : 1.0);
    }
}

