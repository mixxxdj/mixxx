/***************************************************************************
                          ControllerEnumerator.cpp
                       Controller Enumerator Class
                       ----------------------------
    begin                : Sat Apr 30 2011
    copyright            : (C) 2011 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "controllerenumerator.h"

ControllerEnumerator::ControllerEnumerator() : QObject()
{
}

ControllerEnumerator::~ControllerEnumerator() {
    // In this function, the inheriting class must delete the Controllers it creates
}