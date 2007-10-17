/***************************************************************************
                          herculeslinux.h  -  description
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

/*
 * =========Version History=============
 * Version 1.50 Wed Aug 23 2006 - modfications by Garth Dahlstrom <ironstorm@users.sf.net>
 * - works with Hercule DJ Console MK2
 * - reverted pitch control back to pitch knob, added PitchChange method to limit pitch change
 * - fixed volume sliders, crossfader
 * - TODO: reset m_iPitchOffsetLeft or m_iPitchOffsetRight value to -9999 when mouse/keyboard adjusts pitch slider
 *
 */

#ifndef HERCULESLINUX_H
#define HERCULESLINUX_H

#include <q3valuelist.h>
#include "hercules.h"
#include <sys/select.h>
#include <linux/input.h>
#include <qobject.h>

#ifdef __LIBDJCONSOLE__
#include <libdjconsole/djconsole.h>
#endif

/**
  * Linux code for handling the Hercules DJ console.
  *
  *@author Tue Haste Andersen
  *@author Garth Dahlstrom <ironstorm@users.sf.net>
  *  - Added Hercules DJ Console MK2 support to Mixxx 1.5.0 (see also herculeslinux.cpp)
  *  - Applied Hercules Control codes originally mapped out by Eric J. Shattow <shadow@serverart.org> & Oliver M. Bolzer <oliver@fakeroot.net> in their DJConsole-mod.rb ruby script
  */

const int kiHerculesNumValidPrefixes = 3;
static QString kqHerculesValidPrefix[kiHerculesNumValidPrefixes] =
{
    "Hercules Hercules DJ Console Mk2",
    "Hercules Hercules DJ Console",
    "Hercules Hercules DJ Control MP3" // Not sure about this one
};
const int kiHerculesNumEventDevices = 16;

const int kiHerculesLeftTreble = 0x05;
const int kiHerculesLeftMiddle = 0x04;
const int kiHerculesLeftBass = 0x03;
const int kiHerculesLeftVolume = 0x29;
const int kiHerculesLeftPitch = 0x2b;
const int kiHerculesLeftJog = 0x2d;
const int kiHerculesLeftBtnHeadphone = 0x34 + 0x100; // Left Headphones
const int kiHerculesLeftBtnPitchBendMinus = 0x33 + 0x100; // Left PitchDec
const int kiHerculesLeftBtnPitchBendPlus = 0x32 + 0x100; // Left PitchInc
const int kiHerculesLeftBtnTrackPrev = 0x2a + 0x100; // Left TrackDec
const int kiHerculesLeftBtnTrackNext = 0x2b + 0x100; // Left TrackInc
const int kiHerculesLeftBtnCue = 0x28 + 0x100; // Left Cue
const int kiHerculesLeftBtnPlay = 0x27 + 0x100; // Left Play
const int kiHerculesLeftBtnAutobeat = 0x29 + 0x100; // Left AutoBeat
const int kiHerculesLeftBtnMasterTempo = 0x35 + 0x100; // Left Tempo
const int kiHerculesLeftBtn1 = 0x2e + 0x100; // Left Fx1
const int kiHerculesLeftBtn2 = 0x2d + 0x100; // Left Fx2
const int kiHerculesLeftBtn3 = 0x2c + 0x100; // Left Fx3
const int kiHerculesLeftBtnFx = 0x26 + 0x100; // Left FxArrow

const int kiHerculesRightTreble = 0x6;
const int kiHerculesRightMiddle = 0x1;
const int kiHerculesRightBass = 0x0;
const int kiHerculesRightVolume = 0x2a;
const int kiHerculesRightPitch = 0x2c;
const int kiHerculesRightJog = 0x2e;
const int kiHerculesRightBtnHeadphone = 0x38 + 0x100; // Right Headphones
const int kiHerculesRightBtnPitchBendMinus = 0x37 + 0x100; // Right PitchDec
const int kiHerculesRightBtnPitchBendPlus = 0x36 + 0x100; // Right PitchInc
const int kiHerculesRightBtnTrackPrev = 0x24 + 0x100; // Right TrackDec
const int kiHerculesRightBtnTrackNext = 0x25 + 0x100; // Right TrackInc
const int kiHerculesRightBtnCue = 0x22 + 0x100; // Right Cue
const int kiHerculesRightBtnPlay = 0x21 + 0x100; // Right Play
const int kiHerculesRightBtnAutobeat = 0x23 + 0x100; // Right AutoBeat
const int kiHerculesRightBtnMasterTempo = 0x39 + 0x100; // Right Tempo
const int kiHerculesRightBtn1 = 0x2f + 0x100; // Right Fx1
const int kiHerculesRightBtn2 = 0x30 + 0x100; // Right Fx2
const int kiHerculesRightBtn3 = 0x31 + 0x100; // Right Fx3
const int kiHerculesRightBtnFx = 0x20 + 0x100; // Right FxArrow
const int kiHerculesCrossfade = 0x28;

// TODO: 1.5.0+ investigate and revise led codes
const int kiHerculesLedRightSync = 0;
const int kiHerculesLedLeftLoop = 1;
const int kiHerculesLedRightLoop = 2;
const int kiHerculesLedLeftMasterTempo = 3;
const int kiHerculesLedRightMasterTempo = 4;
const int kiHerculesLedLeftFx = 5;
const int kiHerculesLedRightFx = 6;
const int kiHerculesLedRightCueLamp = 7;
const int kiHerculesLedRightCueBtn = 8;
const int kiHerculesLedRightPlay = 9;
const int kiHerculesLedLeftCueLamp = 10;
const int kiHerculesLedLeftPlay = 11;
const int kiHerculesLedLeftHeadphone = 12;
const int kiHerculesLedRightHeadphone = 13;
const int kiHerculesLedLeftCueBtn = 14;
const int kiHerculesLedLeftSync = 15;

#define __THOMAS_HERC__
#ifdef __THOMAS_HERC__
const int kiHerculesHeadphoneSplit = 1;
const int kiHerculesHeadphoneMix = 2;
const int kiHerculesHeadphoneDeckB = 3;
const int kiHerculesHeadphoneDeckA = 4;
#endif

class HerculesLinux : public Hercules
{
public:
    HerculesLinux();
    ~HerculesLinux();
    bool opendev();
    void closedev();
    void getNextEvent();
    void selectMapping(QString qMapping);
#ifdef __LIBDJCONSOLE__
    void consoleEvent(int first, int second);
#endif
protected:
#ifdef __LIBDJCONSOLE__
    DJConsole *djc;
	#ifdef __THOMAS_HERC__
    ControlObject *m_pControlObjectLeftBtnCueAndStop, *m_pControlObjectRightBtnCueAndStop;
    int m_iHerculesHeadphonesSelection;
	#endif //__THOMAS_HERC__
#endif

    void run();  // main thread loop
    int opendev(int iId);
    void led_write(int iLed, bool bOn);

    /** File handle of current open /dev/input/event device */
    int m_iFd;
    /** ID of event interface */
    int m_iId;
    /** List of open devices */
    static Q3ValueList <int> sqlOpenDevs;
    /** File set used in select() call */
    fd_set fdset;
    int m_iJogLeft, m_iJogRight;

    int m_iPitchOffsetLeft, m_iPitchOffsetRight;
    int m_iPitchLeft, m_iPitchRight;


    double m_dJogLeftOld, m_dJogRightOld;

    double PitchChange(const QString ControlSide, const int ev_value, int &m_iPitchPrevious, int &m_iPitchOffset);
};

#endif
