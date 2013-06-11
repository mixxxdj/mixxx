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

/* -------- ------------------------------------------------------
   Purpose: Creates a new rotary encoder
   Input:   key
   -------- ------------------------------------------------------ */
ControlTTRotary::ControlTTRotary(ConfigKey key) : ControlObject(key) {
    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlTTRotaryBehavior());
        delete pOldBehavior;
    }
}



