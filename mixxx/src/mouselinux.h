/***************************************************************************
                          mouselinux.h  -  description
                             -------------------
    begin                : Wed Oct 6 2004
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

#ifndef MOUSELINUX_H
#define MOUSELINUX_H

#include "mouse.h"
#include <q3valuelist.h>
#include <sys/select.h>

/**
  * Linux code for handling Mouse
  *
  *@author Tue Haste Andersen
  */

class QStringList;

const int kiMouseNumDevices = 16;

class MouseLinux : public Mouse
{
public:
    MouseLinux();
    ~MouseLinux();
    bool opendev(QString name);
    void closedev();
    void getNextEvent();
    static QStringList getDeviceList();
protected:

    /** File handle of current open /dev/input/event device */
    int m_iFd;
    fd_set fdset;
    /** True if the last value sent was not a zero */
    bool m_bSending;
};

#endif
