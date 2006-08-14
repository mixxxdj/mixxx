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

#ifndef HERCULESLINUX_H
#define HERCULESLINUX_H

#include <qvaluelist.h>
#include "hercules.h"
#include <sys/select.h>
#include <linux/input.h>
#include <qobject.h>

/**
  * Linux code for handling the Hercules DJ console.
  *
  *@author Tue Haste Andersen
  */

const int kiHerculesNumValidPrefixes = 3;
static QString kqHerculesValidPrefix[kiHerculesNumValidPrefixes] =
{
    "Hercules Hercules DJ Console",
    "Hercules Hercules DJ Console Mk2",
    "Hercules Hercules DJ Control MP3" // Not sure about this one
};
const int kiHerculesNumEventDevices = 16;

const int kiHerculesLeftTreble = 5;
const int kiHerculesLeftMiddle = 4;
const int kiHerculesLeftBass = 3;
const int kiHerculesLeftVolume = 9;
const int kiHerculesLeftPitch = 7;
const int kiHerculesLeftJog = 11;
const int kiHerculesLeftBtnHeadphone = 308;
const int kiHerculesLeftBtnPitchBendMinus = 307;
const int kiHerculesLeftBtnPitchBendPlus = 306;
const int kiHerculesLeftBtnTrackPrev = 298;
const int kiHerculesLeftBtnTrackNext = 299;
const int kiHerculesLeftBtnCue = 296;
const int kiHerculesLeftBtnPlay = 295;
const int kiHerculesLeftBtnAutobeat = 297;
const int kiHerculesLeftBtnMasterTempo = 309;
const int kiHerculesLeftBtn1 = 302;
const int kiHerculesLeftBtn2 = 301;
const int kiHerculesLeftBtn3 = 300;
const int kiHerculesLeftBtnFx = 294;
const int kiHerculesRightTreble = 6;
const int kiHerculesRightMiddle = 1;
const int kiHerculesRightBass = 0;
const int kiHerculesRightVolume = 10;
const int kiHerculesRightPitch = 8;
const int kiHerculesRightJog = 12;
const int kiHerculesRightBtnHeadphone = 312;
const int kiHerculesRightBtnPitchBendMinus = 311;
const int kiHerculesRightBtnPitchBendPlus = 310;
const int kiHerculesRightBtnTrackPrev = 292;
const int kiHerculesRightBtnTrackNext = 293;
const int kiHerculesRightBtnCue = 290;
const int kiHerculesRightBtnPlay = 289;
const int kiHerculesRightBtnAutobeat = 291;
const int kiHerculesRightBtnMasterTempo = 313;
const int kiHerculesRightBtn1 = 303;
const int kiHerculesRightBtn2 = 304;
const int kiHerculesRightBtn3 = 305;
const int kiHerculesRightBtnFx = 288;
const int kiHerculesCrossfade = 2;

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

class HerculesLinux : public Hercules
{
public:
    HerculesLinux();
    ~HerculesLinux();
    bool opendev();
    void closedev();
    void getNextEvent();
    void selectMapping(QString qMapping);

protected:
    void run();  // main thread loop
    int opendev(int iId);
    void led_write(int iLed, bool bOn);

    /** File handle of current open /dev/input/event device */
    int m_iFd;
    /** ID of event interface */
    int m_iId;
    /** List of open devices */
    static QValueList <int> sqlOpenDevs;
    /** File set used in select() call */
    fd_set fdset;
    int m_iJogLeft, m_iJogRight;
    double m_dJogLeftOld, m_dJogRightOld;
};

#endif
