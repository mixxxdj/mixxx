/***************************************************************************
                          controlpushbutton.cpp  -  description
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
#include "controlengine.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new simulated latching push-button. 
   Input:   key - Key for the configuration file
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(ConfigKey key, bool bMidiSimulateLatching) : ControlObject(key)
{
    m_bMidiSimulateLatching = bMidiSimulateLatching;
}

ControlPushButton::~ControlPushButton()
{
}

void ControlPushButton::setValueFromMidi(MidiCategory c, int v)
{
    if (m_bMidiSimulateLatching)
    {
        // Only react on NOTE_ON midi events if simulating latching...
        if (c==NOTE_ON && v>0)
        {
            if (m_dValue==0.)
                m_dValue = 1.;
            else
                m_dValue = 0.;
        }
    }
    else
    {
        if (v==0)
            m_dValue = 0.;
        else
            m_dValue = 1.;
    }
    updateFromMidi();
}

