/***************************************************************************
                          mousewin.h  -  description
                             -------------------
    begin                : Sat Okt 9 2004
    copyright            : (C) 2004 by Tue Haste Andersen
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

#ifndef MOUSEWIN_H
#define MOUSEWIN_H

#include "mouse.h"
#include <objbase.h>
#include <winioctl.h>

#define IOCTL_POWERMATE_SET_LED_BRIGHTNESS CTL_CODE(FILE_DEVICE_UNKNOWN,0x807,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_POWERMATE_PULSE_DURING_SLEEP CTL_CODE(FILE_DEVICE_UNKNOWN,0x808,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_POWERMATE_PULSE_ALWAYS CTL_CODE(FILE_DEVICE_UNKNOWN,0x809,METHOD_BUFFERED,FILE_ANY_ACCESS)
#define IOCTL_POWERMATE_PULSE_SPEED CTL_CODE(FILE_DEVICE_UNKNOWN,0x80A,METHOD_BUFFERED,FILE_ANY_ACCESS)

/**
  * Windows code for handling additional mice.
  *
  *@author Tue Haste Andersen
  */

class MouseWin : public Mouse
{
public:
    MouseWin();
    ~MouseWin();
    bool opendev(QString name);
    void closedev();
    void getNextEvent() {};
protected:
    void run();
    void led_write(int iStaticBrightness, int iSpeed, int iTable, int iAsleep, int iAwake);
    HANDLE GetDeviceViaInterface(GUID* pGuid, DWORD instance);

    /** File handle of current open /dev/input/event device */
    HANDLE m_hFd;
    /** ID of event interface */
    int m_iId;
};

#endif


