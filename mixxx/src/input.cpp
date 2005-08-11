/***************************************************************************
                          rotary.cpp  -  description
                             -------------------
    begin                : Thu Feb 24 2005
    copyright            : (C) 2005 by Tue Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qapplication.h"
#include "input.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "midiobject.h"
#include "mathstuff.h"

Input::Input()
{
}

Input::~Input()
{
    if (running())
    {
        terminate();
        wait();
    }
}

/*
QStringList Input::getMappings()
{
    return QStringList();
}

void Input::setMapping(QString mapping)
{
}
*/

void Input::run()
{
    while (1)
        getNextEvent();
}

void Input::sendEvent(double dValue, ControlObject *pControlObject)
{
    if (pControlObject)
        pControlObject->queueFromMidi(CTRL_CHANGE, dValue);
}

void Input::sendButtonEvent(bool bPressed, ControlObject *pControlObject)
{
    if (pControlObject)
    {
        if (bPressed)
            pControlObject->queueFromMidi(NOTE_ON, 1);
        else
            pControlObject->queueFromMidi(NOTE_OFF, 0);
    }
}
