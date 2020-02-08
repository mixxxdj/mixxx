/***************************************************************************
                          controlplaybutton.cpp  -  description
                             -------------------
    begin                : Sat Feb 29 2020
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

#include "control/controlplaybutton.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new simulated latching push-button.
   Input:   key - Key for the configuration file
   -------- ------------------------------------------------------ */
ControlPlayButton::ControlPlayButton(QString group)
        : ControlPushButton(ConfigKey(group, "play")) {
    setButtonMode(ControlPushButton::TOGGLE);
    setStates(3);

    // Overwrite previously set PushButtonBehavior with custom PlayButtonBehavior
    if (m_pControl) {
        m_pControl->setBehavior(new ControlPlayButtonBehavior());
    }
}

ControlPlayButton::~ControlPlayButton() {
}
