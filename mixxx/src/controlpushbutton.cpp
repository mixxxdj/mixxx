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

// static
const int ControlPushButton::kPowerWindowTimeMillis = 300;

/* -------- ------------------------------------------------------
   Purpose: Creates a new simulated latching push-button.
   Input:   key - Key for the configuration file
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(ConfigKey key) :
        ControlObject(key, false),
        m_buttonMode(PUSH),
        m_iNoStates(2) {
}

ControlPushButton::~ControlPushButton() {
}

// Tell this PushButton how to act on rising and falling edges
void ControlPushButton::setButtonMode(enum ButtonMode mode) {
    //qDebug() << "Setting " << m_Key.group << m_Key.item << "as toggle";
    m_buttonMode = mode;
}

void ControlPushButton::setStates(int num_states) {
    m_iNoStates = num_states;
}

void ControlPushButton::setValueFromMidi(MidiOpCode o, double v) {
    // keyboard events are handled by this function as well
    //if (m_bMidiSimulateLatching)

    //qDebug() << "bMidiSimulateLatching is true!";
    // Only react on NOTE_ON midi events if simulating latching...

    //qDebug() << o << v;

    // This block makes push-buttons act as toggle buttons.
    if (m_buttonMode == POWERWINDOW && m_iNoStates == 2) {
         if (o == MIDI_NOTE_ON) {
             if (v > 0.) {
                 m_dValue = !m_dValue;
                 m_pushTimer.setSingleShot(true);
                 m_pushTimer.start(kPowerWindowTimeMillis);
             }
         } else if (o == MIDI_NOTE_OFF) {
             if (!m_pushTimer.isActive()) {
                 m_dValue = 0.0;
             }
         }
    } else if (m_buttonMode == TOGGLE) {
        // This block makes push-buttons act as toggle buttons.
        if (m_iNoStates > 2) { //multistate button
            if (v > 0.) { //looking for NOTE_ON doesn't seem to work...
                m_dValue++;
                if (m_dValue >= m_iNoStates)
                    m_dValue = 0;
            }
        } else {
            if (o == MIDI_NOTE_ON) {
                if (v > 0.) {
                    m_dValue = !m_dValue;
                }
            }
        }
    } else { //Not a toggle button (trigger only when button pushed)
        if (o == MIDI_NOTE_ON) {
            m_dValue = !m_dValue;
        } else if (o == MIDI_NOTE_OFF) {
            m_dValue = 0.0;
        }
    }

    emit(valueChanged(m_dValue));
}

