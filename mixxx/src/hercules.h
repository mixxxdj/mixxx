/***************************************************************************
                          hercules.h  -  description
                             -------------------
    begin                : Tue Feb 22 2005
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

#ifndef HERCULES_H
#define HERCULES_H

#include <qsemaphore.h>
#include "input.h"

/**
  * Virtual class for handling the Hercules DJ Console. This is implemented as a separate thread.
  *
  *@author Tue Haste Andersen
  */

//const int kiPowermateBufferSize = 32;
//const int kiPowermateKnobIntegralMaxLen = 25;
class ControlObject;
class Rotary;

class Hercules : public Input
{
public:
    Hercules();
    ~Hercules();
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
    
    ControlObject *m_pControlObjectLeftTreble, *m_pControlObjectLeftMiddle, *m_pControlObjectLeftBass,
                  *m_pControlObjectLeftVolume, *m_pControlObjectLeftPitch, *m_pControlObjectLeftJog,
                  *m_pControlObjectLeftBtnHeadphone, *m_pControlObjectLeftBtnPitchBendMinus, 
                  *m_pControlObjectLeftBtnPitchBendPlus, *m_pControlObjectLeftBtnTrackPrev,
                  *m_pControlObjectLeftBtnTrackNext, *m_pControlObjectLeftBtnCue, *m_pControlObjectLeftBtnPlay,
                  *m_pControlObjectLeftBtnAutobeat, *m_pControlObjectLeftBtnMasterTempo, *m_pControlObjectLeftBtn1,
                  *m_pControlObjectLeftBtn2, *m_pControlObjectLeftBtn3, *m_pControlObjectLeftBtnFx;
    ControlObject *m_pControlObjectRightTreble, *m_pControlObjectRightMiddle, *m_pControlObjectRightBass,
                  *m_pControlObjectRightVolume, *m_pControlObjectRightPitch, *m_pControlObjectRightJog,
                  *m_pControlObjectRightBtnHeadphone, *m_pControlObjectRightBtnPitchBendMinus, 
                  *m_pControlObjectRightBtnPitchBendPlus, *m_pControlObjectRightBtnTrackPrev,
                  *m_pControlObjectRightBtnTrackNext, *m_pControlObjectRightBtnCue, *m_pControlObjectRightBtnPlay,
                  *m_pControlObjectRightBtnAutobeat, *m_pControlObjectRightBtnMasterTempo, *m_pControlObjectRightBtn1,
                  *m_pControlObjectRightBtn2, *m_pControlObjectRightBtn3, *m_pControlObjectRightBtnFx;
    ControlObject *m_pControlObjectCrossfade;   
    Rotary *m_pRotaryLeft, *m_pRotaryRight; 
};

#endif
