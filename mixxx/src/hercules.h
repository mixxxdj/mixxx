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

#ifdef __WIN__
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

/**
  * Virtual class for handling the Hercules DJ Console. This is implemented as a separate thread.
  *
  *@author Tue Haste Andersen
  */


const QString kqInputMappingHerculesStandard = "Standard";
const QString kqInputMappingHerculesInBeat = "InBeat";

//const int kiPowermateBufferSize = 32;
//const int kiPowermateKnobIntegralMaxLen = 25;
class ControlObject;
class ControlObjectThread;
class Rotary;

class Hercules : public Input
{
public:
    Hercules();
    ~Hercules();
    /** Return a list of available mappings */
    static QStringList getMappings();
    /** Select mapping */
    virtual void selectMapping(QString qMapping);
    bool opendev(QString) { return opendev(); }
    virtual bool opendev() = 0;
    void led(); //int iLedNo, bool bOn);

    ControlObjectThread *m_pControlObjectLeftTreble, *m_pControlObjectLeftMiddle, *m_pControlObjectLeftBass,
                        *m_pControlObjectLeftVolume, *m_pControlObjectLeftPitch, *m_pControlObjectLeftJog,
                        *m_pControlObjectLeftBtnHeadphone, *m_pControlObjectLeftBtnPitchBendMinus,
                        *m_pControlObjectLeftBtnPitchBendPlus, *m_pControlObjectLeftBtnTrackPrev,
                        *m_pControlObjectLeftBtnTrackNext, *m_pControlObjectLeftBtnCue, *m_pControlObjectLeftBtnPlay,
                        *m_pControlObjectLeftBtnAutobeat, *m_pControlObjectLeftBtnMasterTempo, *m_pControlObjectLeftBtn1,
                        *m_pControlObjectLeftBtn2, *m_pControlObjectLeftBtn3, *m_pControlObjectLeftBtnFx;
    ControlObjectThread *m_pControlObjectRightTreble, *m_pControlObjectRightMiddle, *m_pControlObjectRightBass,
                        *m_pControlObjectRightVolume, *m_pControlObjectRightPitch, *m_pControlObjectRightJog,
                        *m_pControlObjectRightBtnHeadphone, *m_pControlObjectRightBtnPitchBendMinus,
                        *m_pControlObjectRightBtnPitchBendPlus, *m_pControlObjectRightBtnTrackPrev,
                        *m_pControlObjectRightBtnTrackNext, *m_pControlObjectRightBtnCue, *m_pControlObjectRightBtnPlay,
                        *m_pControlObjectRightBtnAutobeat, *m_pControlObjectRightBtnMasterTempo, *m_pControlObjectRightBtn1,
                        *m_pControlObjectRightBtn2, *m_pControlObjectRightBtn3, *m_pControlObjectRightBtnFx;
    ControlObjectThread *m_pControlObjectLeftBtn123[3], *m_pControlObjectRightBtn123[3];
    ControlObjectThread *m_pControlObjectCrossfade, *m_pControlObjectMainVolume, *m_pControlObjectBalance;
    ControlObjectThread *m_pControlObjectGainA, *m_pControlObjectGainB;
    ControlObjectThread *m_pControlObjectLeftKillHigh, *m_pControlObjectRightKillHigh;
    ControlObjectThread *m_pControlObjectLeftKillMid, *m_pControlObjectRightKillMid;
    ControlObjectThread *m_pControlObjectLeftKillBass, *m_pControlObjectRightKillBass;

    ControlObjectThread *m_pControlObjectUp, *m_pControlObjectDown;
    ControlObjectThread *m_pControlObjectLeft, *m_pControlObjectRight;

    ControlObjectThread *m_pControlObjectLoadDeckA, *m_pControlObjectLoadDeckB;

    ControlObjectThread *m_pControlObjectLeftScratch, *m_pControlObjectRightScratch;

protected:
    /** Change jog mode */
    void changeJogMode(int iLeftFxMode=0, int iRightFxMode=0);
    /** Change the led */
    //virtual void led_write(int static_brightness, int speed, int table, int asleep, int awake) = 0;
    /** Instantiate number. Used in the calculation of MIDI controller id's */
    int m_iInstNo;
    /** Pointer to semaphore used to control led */
    #ifdef QT3_SUPPORT
    Q3Semaphore m_qRequestLed;
    #else
    QSemaphore m_qRequestLed;
    #endif

    ControlObjectThread *m_pControlObjectLeftBtnPlayProxy, *m_pControlObjectRightBtnPlayProxy,
                        *m_pControlObjectLeftBtnLoopProxy, *m_pControlObjectRightBtnLoopProxy,
                        *m_pControlObjectLeftBtnHeadphoneProxy, *m_pControlObjectRightBtnHeadphoneProxy;

    Rotary *m_pRotaryLeft, *m_pRotaryRight;
    ControlObjectThread *m_pControlObjectLeftBeatLoop, *m_pControlObjectRightBeatLoop;
    ControlObjectThread *m_pControlObjectLeftVuMeter,  *m_pControlObjectRightVuMeter;
    QString m_qMapping;


    // Various states maintained locally in this object:
    int m_iLeftFxMode, m_iRightFxMode;
    bool m_bPlayLeft, m_bPlayRight, m_bCueLeft, m_bCueRight, m_bLoopLeft, m_bLoopRight,
         m_bMasterTempoLeft, m_bMasterTempoRight, m_bHeadphoneLeft, m_bHeadphoneRight,
         m_bSyncLeft, m_bSyncRight;

    bool scratchMode;

    double m_dLeftVolumeOld, m_dRightVolumeOld;
    int m_iLeftVU, m_iRightVU;
};

#endif
