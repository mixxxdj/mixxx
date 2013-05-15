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

/* -------- ------------------------------------------------------
   Purpose: Creates a new simulated latching push-button.
   Input:   key - Key for the configuration file
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(ConfigKey key) :
        ControlObject(key, false),
        m_buttonMode(PUSH),
        m_iNoStates(2) {
    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlPushButtonBehavior(
                static_cast<ControlPushButtonBehavior::ButtonMode>(m_buttonMode),
                m_iNoStates));
        delete pOldBehavior;
    }
}

ControlPushButton::~ControlPushButton() {
}

// Tell this PushButton how to act on rising and falling edges
void ControlPushButton::setButtonMode(enum ButtonMode mode) {
    //qDebug() << "Setting " << m_Key.group << m_Key.item << "as toggle";
    m_buttonMode = mode;

    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlPushButtonBehavior(
                static_cast<ControlPushButtonBehavior::ButtonMode>(m_buttonMode),
                m_iNoStates));
        delete pOldBehavior;
    }
}

void ControlPushButton::setStates(int num_states) {
    m_iNoStates = num_states;

    if (m_pControl) {
        ControlNumericBehavior* pOldBehavior = m_pControl->setBehavior(
            new ControlPushButtonBehavior(
                static_cast<ControlPushButtonBehavior::ButtonMode>(m_buttonMode),
                m_iNoStates));
        delete pOldBehavior;
    }
}

