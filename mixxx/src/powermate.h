/***************************************************************************
                          powermate.h  -  description
                             -------------------
    begin                : Tue Apr 29 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef POWERMATE_H
#define POWERMATE_H

#include <qsemaphore.h>
#include "input.h"

class Rotary;
class ControlObject;

/**
  * Virtual class for handling the PowerMate. This is implemented as a separate thread.
  *
  *@author Tue & Ken Haste Andersen
  */

const int kiPowermateBufferSize = 32;
const int kiPowermateKnobIntegralMaxLen = 25;

class PowerMate : public Input
{
public:
    PowerMate();
    ~PowerMate();
    bool opendev(QString) { return opendev(); }
    virtual bool opendev() = 0;
    void led();
protected:
    /** Change the led */
    virtual void led_write(int static_brightness, int speed, int table, int asleep, int awake) = 0;
    /** Instantiate number. Used in the calculation of MIDI controller id's */
    int m_iInstNo;
    /** Pointer to semaphore used to control led */
    QSemaphore *m_pRequestLed;
    /** Pointer to rotary object */
    Rotary *m_pRotary;
    /** Pointer to control objects connected to the PowerMate */
    ControlObject *m_pControlObjectRotary, *m_pControlObjectButton;
};

#endif
