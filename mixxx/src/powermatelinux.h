/***************************************************************************
                          powermatelinux.h  -  description
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

#ifndef POWERMATELINUX_H
#define POWERMATELINUX_H

#include <qvaluelist.h>
#include "powermate.h"
/**
  * Linux code for handling the PowerMate.
  *
  *@author Tue & Ken Haste Andersen
  */

const int kiPowermateNumValidPrefixes = 2;
static const char *pPowermateValidPrefix[kiPowermateNumValidPrefixes] =
{
    "Griffin PowerMate",
    "Griffin SoundKnob"
};
const int kiPowermateNumEventDevices = 16;

class PowerMateLinux : public PowerMate
{
public:
    PowerMateLinux();
    ~PowerMateLinux();
    int opendev(int iId);
    void closedev();
protected:
    bool opendev();
    void run();
    void led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake);
    void process_event(struct input_event *pEv);

    /** File handle of current open /dev/input/event device */
    int m_iFd;
    /** ID of event interface */
    int m_iId;
    /** List of open devices */
    static QValueList <int> sqlOpenDevs;
};

#endif
