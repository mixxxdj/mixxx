/***************************************************************************
                          joystick.h  -  description
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

#ifndef JOYSTICK_H
#define JOYSTICK_H

/**
  * Joystick.c - gameport/joystick-support for mixxx
  *              converts joystick-input into internal midi messages 
  *@author Svein Magne Bang
  */


#include <qsemaphore.h>
#include <qthread.h>
 
class ControlObject;
    
const int kiJoystickMidiChannel=18;  // internal midi channel (+1 in real life)

typedef struct {
    char name[255];
    int axes;
    int buttons;
} joystickType;

  
class Joystick : public QThread
{
public: 
    Joystick();
    ~Joystick();
    virtual int opendev() = 0;
    virtual void closedev() = 0;
protected:
    virtual void run() = 0;    // Main thread loop
    joystickType joystick;
    ControlObject *m_pControl;    // Pointer to ControlObject
};

#endif
