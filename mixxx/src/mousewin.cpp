/***************************************************************************
                          mousewin.cpp  -  description
                             -------------------
    begin                : Sat Oct 9 2004
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

#include "mousewin.h"
#include "controlobject.h"
#include "mathstuff.h"
//#include <setupapi.h>
//#include <initguid.h>
#include <qstring.h>

MouseWin::MouseWin() : Mouse()
{
	if (FAILED(DirectInput8Create((HINSTANCE)GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8,
								  (void **)&g_lpDI, NULL)))
		qDebug("DirectInput not available");
	
    // Obtain an interface to the system mouse device. 
    if (FAILED(g_lpDI->CreateDevice(GUID_SysMouse, &g_pMouse, NULL)) 
		qDebug("Could not obtain pointer to mouse device");

    //  Set the data format to "mouse format". 
    if (FAILED(g_pMouse->SetDataFormat(&c_dfDIMouse))
		qDebug("Could not set data format for mouse");

	if (FAILED(g_pMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
		qDebug("Could not set cooperative level for mouse");

}

MouseWin::~MouseWin()
{
}

bool MouseWin::opendev(QString name)
{

    // Start thread
    start();

    return true;
}

void MouseWin::run()
{
}

void MouseWin::closedev()
{
}

void MouseWin::getNextEvent()
{
}
