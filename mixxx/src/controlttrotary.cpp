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
    value = 0.;
    received = 0;
    
    // Connect 10ms timer, and start
    //connect(&timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    //timer.start(100,false);
}

void ControlTTRotary::setValue(int v)
{
    value = ((float)v-64.)/200.;
    emit(updateGUI(v));
}

void ControlTTRotary::slotSetPosition(int v)
{
    value = ((float)v-64.)/200.;
    qDebug("%f",value);

    emitValueChanged(value);
/*    if (v==0)
        received--;
    else
        received++;
*/
}

void ControlTTRotary::slotSetPositionMidi(MidiCategory, int v)
{
    //qDebug("thread id: %p",pthread_self());

    slotSetPosition(v);
    emit(updateGUI(v));
}

FLOAT_TYPE ControlTTRotary::getValue()
{
    return value;
}

/*
void ControlTTRotary::slotSetValue(int newvalue)
{
    value = ((FLOAT_TYPE)newvalue-64.)/200.;
    emitValueChanged(value);
}
*/
/*
void ControlTTRotary::slotTimer()
{
    FLOAT_TYPE newv = (FLOAT_TYPE)received/1000.;
    received = 0;
    
    if (newv != value)
    {
        value = newv;

        emitValueChanged(value);
        //updateGUI();
    }
    //qDebug("rotary: %f",value);
}
*/

void ControlTTRotary::forceGUIUpdate()
{
    emit(updateGUI((int)(value*200)+64));
}
