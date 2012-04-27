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

#include <math.h>
#include "controllogpotmeter.h"

#define maxPosition 127
#define minPosition 0
#define middlePosition ((maxPosition-minPosition)/2)
#define positionrange (maxPosition-minPosition)

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

    m_dValue = 1.0;
    m_dDefaultValue = 1.0;
}

double ControlLogpotmeter::getValueFromWidget(double dValue)
{
    double dResult = 0;

    // Calculate the value linearly:
    if (!m_bTwoState)
    {
        dResult = pow(10., (double)(m_fB1*dValue)) - 1;
    }
    else
    {
        if (dValue <= middlePosition)
            dResult = pow(10., m_fB1*dValue) - 1;
        else
            dResult = pow(10., m_fB2*(dValue - middlePosition));
    }

    //qDebug() << "Midi: " << dValue << " ValueFromWidget : " << m_dValue;
    return dResult;
}

double ControlLogpotmeter::getValueToWidget(double dValue)
{
    double pos;

    if (!m_bTwoState)
    {
        pos = log10(dValue+1)/m_fB1;
    }
    else
    {
        if (m_dValue>1.)
            pos = log10(dValue)/m_fB2 + middlePosition;
        else
            pos = log10(dValue+1)/m_fB1;
    }
    //qDebug() << "GetValueToWidget : " << pos;
    return pos;
}

double ControlLogpotmeter::GetMidiValue()
{
    double midival = 0.;

    midival = getValueToWidget(m_dValue);
    //    midival = 127.*(midival-m_dMinValue)/m_dValueRange
    //qDebug() << "GetMidiValue : " << midival;
    return midival;
}

void ControlLogpotmeter::setValueFromMidi(MidiOpCode o, double v) {
    Q_UNUSED(o);
  //    m_dValue = m_dMinValue + (v/127.)*m_dValueRange;
    m_dValue = getValueFromWidget(v);
    //    qDebug() << "SetValueFromMidiValue : " << m_dValue;
    emit(valueChanged(m_dValue));
}

