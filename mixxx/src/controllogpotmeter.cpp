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

#include "controllogpotmeter.h"
#include "controlengine.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new logarithmic potmeter, where the value is
            given by: value = a*10^(b*midibyte) - 1. The lower value
            is 0, for midibyte=64 the value is 1 and the upper 
            value is set by maxvalue.
   Input:   n - name
            midino - number of the midi controller.
            midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlLogpotmeter::ControlLogpotmeter(ConfigKey key, double dMaxValue) : ControlPotmeter(key)
{
    a = 1.;
    b = log10(2.)/middlePosition;
    b2 = log10((dMaxValue+1.)/2.)/middlePosition;
    a2 = 2.*pow(10., -middlePosition*b);

    m_dMinValue = 0.;
    m_dMaxValue = dMaxValue;
    m_dValueRange = m_dMaxValue-m_dMinValue;
        
    m_dValue = 1.;
}

//#define min(a,b)            (((a) < (b)) ? (a) : (b))
void ControlLogpotmeter::setValueFromWidget(double dValue)
{
    // Calculate the value linearly:
    if (dValue <= middlePosition)
        m_dValue = a*pow(10., b*dValue) - 1.;
    else
        m_dValue = a2*pow(10., b2*dValue) - 2.;

    updateFromWidget();
}

void ControlLogpotmeter::updateWidget()
{
    double pos;
    if (m_dValue>1.)
        pos = log10(m_dValue+2./a2)/b2;
    else
        pos = log10(m_dValue+1./a)/b;    

    emit(signalUpdateWidget(pos));
}

