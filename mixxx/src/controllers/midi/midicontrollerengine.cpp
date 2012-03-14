/**
* @file midicontrollerengine.cpp
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 13 Mar 2012
* @brief Controller engine extensions for MIDI controllers
*
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "midicontrollerengine.h"

MidiControllerEngine::MidiControllerEngine(MidiController* controller) : ControllerEngine((Controller*)controller) {
}

MidiControllerEngine::~MidiControllerEngine() {
}

/* -------- ------------------------------------------------------
   Purpose: Evaluate & call a script function
   Input:   Function name, status, control #, value, ControlObject group
   Output:  false if an invalid function or an exception
   -------- ------------------------------------------------------ */
bool MidiControllerEngine::execute(QString function, char status,
                                   char control, char value,
                                   QString group) {

    if(m_pEngine == NULL) {
        return false;
    }

    QScriptValue scriptFunction = m_pEngine->evaluate(function);

    if (checkException())
        return false;
    if (!scriptFunction.isFunction())
        return false;

    QScriptValueList args;
    args << QScriptValue(status & 0x0F);
    args << QScriptValue(control);
    args << QScriptValue(value);
    args << QScriptValue(status);
    args << QScriptValue(group);

    scriptFunction.call(QScriptValue(), args);
    if (checkException())
        return false;
    return true;
}