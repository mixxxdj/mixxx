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

#include <QtCore>
#include "input.h"
#include "controlobject.h"
#include "controlobjectthreadmain.h"
#include "controlobjectthread.h"
#include "mathstuff.h"

// TODO: Investigate the use of ControlObjectThread vs. ControlObjectThreadMain
//       (the #include "controlobjectthreadmain.h" was already here before I fixed
//        the queueFromThread mess. I wonder if this file was unfinished when Tue
//        left...) - Albert Nov 5, 2007

Input::Input()
{
}

Input::~Input()
{
    if (isRunning())
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
    unsigned static id = 0; //the id of this thread, for debugging purposes //XXX copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("Input %1").arg(++id));
    
    while (1)
        getNextEvent();
}

void Input::sendEvent(double dValue, ControlObject * pControlObject)
{
    if (pControlObject)
        pControlObject->queueFromMidi(CTRL_CHANGE, dValue);
}

void Input::sendEvent(double dValue, ControlObjectThread *pControlObjThread)
{
    if (pControlObjThread)
        pControlObjThread->slotSet(dValue);
}

void Input::sendButtonEvent(bool bPressed, ControlObject * pControlObject)
{
    if (pControlObject)
    {
        if (bPressed)
            pControlObject->queueFromMidi(NOTE_ON, 1);
        else
            pControlObject->queueFromMidi(NOTE_OFF, 1);
    }
}

void Input::sendButtonEvent(bool bPressed, ControlObjectThread *pControlObjThread)
{
    if (pControlObjThread)
    {
        if (bPressed)
            pControlObjThread->slotSet(1.0f);
        else
            pControlObjThread->slotSet(0.0f);
    }
}
