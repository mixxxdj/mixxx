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
            given by: 
            
                value = 10^(b*midibyte) - 1
            
            The lower value is 0, for midibyte=64 the value is 1 and the upper 
            value is set by maxvalue.

            If the maxvalue is set to 1, the potmeter operates with only
            one logarithmid scale between 0 (for midi 0) and 1 (midivalue 128).
   Input:   n - name
            midino - number of the midi controller.
            midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlLogpotmeter::ControlLogpotmeter(ConfigKey key, double dMaxValue) : ControlPotmeter(key)
{
    m_dMinValue = 0.;
    m_dMaxValue = dMaxValue;

    if (m_dMaxValue==1.)
    {
        m_bTwoState = false;

        m_fB1 = log10(2.)/maxPosition;
    }
    else
    {
        m_bTwoState = true;

        m_fB1 = log10(2.)/middlePosition;
        m_fB2 = log10(dMaxValue)/(maxPosition-middlePosition);
    }
    
    m_dValueRange = m_dMaxValue-m_dMinValue;
        
    m_dValue = 1.;
}

void ControlLogpotmeter::setValueFromWidget(double dValue)
{
    // Calculate the value linearly:
    if (!m_bTwoState)
    {
        m_dValue = pow(10, m_fB1*dValue) - 1;
    }
    else
    {
        if (dValue <= middlePosition)
            m_dValue = pow(10., m_fB1*dValue) - 1;
        else
            m_dValue = pow(10., m_fB2*(dValue - middlePosition));
    }

    qDebug("Midi: %f Value : %f", dValue, m_dValue);

    updateFromWidget();
}

void ControlLogpotmeter::updateWidget()
{
    double pos;

    if (!m_bTwoState)
    {
        pos = log10(m_dValue+1)/m_fB1;
    }
    else
    {
        if (m_dValue>1.)
            pos = log10(m_dValue)/m_fB2 + middlePosition;
        else
            pos = log10(m_dValue+1)/m_fB1;    
    }
    
    emit(signalUpdateWidget(pos));
}

