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

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

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
    void getNextEvent();

protected:
	/** Pointer to DirectInput object */
	LPDIRECTINPUT8 g_lpDI;
    /** Pointer to mouse device */
	LPDIRECTINPUTDEVICE g_pMouse; 
};

#endif


