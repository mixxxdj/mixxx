/***************************************************************************
                          powermatewin.h  -  description
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

#ifndef POWERMATEWIN_H
#define POWERMATEWIN_H

#include "powermate.h"
#include <objbase.h>
#include <winioctl.h>

#define IOCTL_POWERMATE_SET_LED_BRIGHTNESS CTL_CODE(FILE_DEVICE_UNKNOWN,0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_POWERMATE_PULSE_DURING_SLEEP CTL_CODE(FILE_DEVICE_UNKNOWN,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_POWERMATE_PULSE_ALWAYS CTL_CODE(FILE_DEVICE_UNKNOWN,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_POWERMATE_PULSE_SPEED CTL_CODE(FILE_DEVICE_UNKNOWN,0x80A,METHOD_BUFFERED,FILE_ANY_ACCESS)
 
/**
  * Windows code for handling the PowerMate.
  *
  *@author Tue & Ken Haste Andersen
  */

class PowerMateWin : public PowerMate 
{
public: 
    PowerMateWin(ControlObject *pControl);
    ~PowerMateWin();
    bool opendev();
    void closedev();
protected:
    void run();
    void led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake);
    void process_event(char *pEv);
	HANDLE GetDeviceViaInterface(GUID* pGuid, DWORD instance);

    /** File handle of current open /dev/input/event device */
    HANDLE m_hFd;
    /** ID of event interface */
    int m_iId;
    /** Count instantiated devices */
    static int siInstCount;
};

#endif


