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

#include <qthread.h>
#include <qsemaphore.h>

/**
  * Virtual class for handling the PowerMate. This is implemented as a separate thread.
  *
  *@author Tue & Ken Haste Andersen
  */

class ControlObject;

  
// Parameters used to interface the PowerMate with the MIDI code
const int kiPowermateMidiChannel=16;
const int kiPowermateMidiDial = 1;
const int kiPowermateMidiBtn = 2;

const int kiPowermateBufferSize = 32;
const int kiPowermateKnobIntegralLen = 25;

class PowerMate : public QThread
{
public: 
    PowerMate(ControlObject *pControl);
    ~PowerMate();
    void led();
    /** Open a PowerMate device */
    virtual bool opendev() = 0;
    /** Close the device */
    virtual void closedev() = 0;
protected:
    /** Main thread loop */ 
    virtual void run() = 0;
    /** Change the led */
    virtual void led_write(int static_brightness, int speed, int table, int asleep, int awake) = 0;
    /** This method should be called every 50ms during an active period of the knob. The method sends
      * knob events out, which has been interpolated and fitted to a certain function */
    void knob_event();

    /** Instantiate number. Used in the calculation of MIDI controller id's */
    int m_iInstNo;
    /** Pointer to ControlObject */
    ControlObject *m_pControl;
    /** Variable used to indicate weather a knob event should be sent or not */
    bool m_bSendKnobEvent;
    /** Amplitude of knob */
    float m_fMagnitude;
    /** Current value got from knob */
    int m_iKnobVal;
    /** Pointer to knob integral buffer */
    int *m_pKnobIntegral;
    /** Pointer to semaphore used to control led */
    QSemaphore *m_pRequestLed;
};

#endif
