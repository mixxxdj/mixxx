/***************************************************************************
                          controlttrotary.cpp  -  description
                             -------------------
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

#include "controlttrotary.h"
#include "controlengine.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new rotary encoder
   Input:   key
   -------- ------------------------------------------------------ */
ControlTTRotary::ControlTTRotary(ConfigKey key) : ControlObject(key)
{
}

void ControlTTRotary::setValueFromWidget(double dValue)
{
    // Non-linear scaling
    double temp = (((dValue-64.)*(dValue-64.))/64.)/100.;
    if ((temp-64.)<0)
        m_dValue = -temp;
    else
        m_dValue = temp;

    updateFromWidget();
}

void ControlTTRotary::setValueFromMidi(MidiCategory, int v)
{
    double temp = ((((double)v-64.)*((double)v-64.))/64.)/100.;
    if ((temp-64.)<0)
        m_dValue = -temp;
    else
        m_dValue = temp;

    updateFromMidi();        
}

void ControlTTRotary::updateWidget()
{
    emit(signalUpdateWidget(m_dValue*200.+64.));
}
