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

/* -------- ------------------------------------------------------
   Purpose: Creates a new potmeter
   Input:   n - name
            midino - number of the midi controller.
            master - pointer to the control to which the potmeter is
                     attached. This control is acknowledged when the
                     potmeter is changed.
            midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlPotmeter::ControlPotmeter(ConfigKey key, FLOAT_TYPE _minvalue, FLOAT_TYPE _maxvalue) : ControlObject(key)
{
    setRange(_minvalue,_maxvalue);
}

ControlPotmeter::~ControlPotmeter()
{
}

/* -------- ------------------------------------------------------
   Purpose: Set the position of the potmeter, and change the
            value correspondingly.
   Input:   The (new) position.
   Output:  The value is updated.
   -------- ------------------------------------------------------ */
void ControlPotmeter::slotSetPositionExtern(float newpos)
{
    value = minvalue + ((FLOAT_TYPE)newpos/127.)*valuerange;

    emitValueChanged(value);
}

void ControlPotmeter::slotSetPositionMidi(MidiCategory c, int v)
{
    slotSetPositionExtern(v);
    emit(updateGUI(v));
}

void ControlPotmeter::forceGUIUpdate()
{
    emit(updateGUI((int)(127.*(value-minvalue)/valuerange)));
}

void ControlPotmeter::setValue(int newpos)
{
    value = minvalue + ((FLOAT_TYPE)newpos/127.)*valuerange;;
    emit(updateGUI(newpos));
}

FLOAT_TYPE ControlPotmeter::getValue()
{
    return value;
}

float ControlPotmeter::getMin()
{
    return minvalue;
}

float ControlPotmeter::getMax()
{
    return maxvalue;
}

void ControlPotmeter::setRange(float fMin, float fMax)
{
    minvalue = fMin;
    maxvalue = fMax;
    valuerange = maxvalue-minvalue;
    value = minvalue + 0.5*valuerange;
    emitValueChanged(value);
    forceGUIUpdate();
}

