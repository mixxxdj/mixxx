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

#ifdef __WIN32__
#ifndef _MSC_VER
/* Not MSVC, one of these:
   __MINGW32__
   __CYGWIN__ 
*/
#include <windows.h>
#endif
#endif

#ifdef QT3_SUPPORT
#include <q3semaphore.h>
#else
#include <q3semaphore.h>
#endif

#include "input.h"

class Rotary;
class ControlObject;
class ControlObjectThreadMain;

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
    static QStringList getMappings();
    void selectMapping(QString mapping);
    bool opendev(QString) { return opendev(); }
    virtual bool opendev() = 0;
    void led();
protected:
    /** Change the led */
    virtual void led_write(int static_brightness, int speed, int table, int asleep, int awake) = 0;
    /** Instantiate number. Used in the calculation of MIDI controller id's */
    int m_iInstNo;
    /** Semaphore used to control led */
    #ifdef QT3_SUPPORT
    Q3Semaphore m_qRequestLed;
    #else
    QSemaphore m_qRequestLed;
    #endif
    /** Pointer to rotary object */
    Rotary *m_pRotary;
    /** Pointer to control objects connected to the PowerMate */
    ControlObjectThreadMain *m_pControlObjectRotary, *m_pControlObjectButton;
    ControlObjectThreadMain *m_ctrlVuMeter; //A pointer to a VuMeter, so we can sync the powermate's LED with the music.
};

#endif
