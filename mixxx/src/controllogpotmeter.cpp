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
ControlLogpotmeter::ControlLogpotmeter(ConfigKey key, double dMaxValue)
    : ControlPotmeter(key, 0, dMaxValue) {
    // Override ControlPotmeters default value of 0.5
    setDefaultValue(1.0);
    set(1.0);

    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlLogpotmeterBehavior(dMaxValue));
        delete pOldBehavior;
    }
}

