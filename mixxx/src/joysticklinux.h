/***************************************************************************
                          joysticklinux.h  -  description
                             -------------------
    begin                : Thu Jul 10 2003
    copyright            : (C) 2003 by Svein Magne Bang
    email                : sveinmb@stud.ntnu.no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef JOYSTICKLINUX_H
#define JOYSTICKLINUX_H

#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <joystick.h>

/**
  *@author Svein Magne Bang
  */

class JoystickLinux : public Joystick  
{
public: 
    JoystickLinux(ControlObject *pControl);
    ~JoystickLinux();
    int opendev();
    void closedev();
protected:
    void run();  // main thread loop
    int joystickDevice;
    struct js_event joystickEvent;
    int axisvalue[8];
    int buttonvalue[8];
    int value;
};

#endif
