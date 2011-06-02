/**
* @file controllerenumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief Base class handling discovery and enumeration of DJ controllers.
*
* This class handles discovery and enumeration of DJ controllers and 
*   must be inherited by a class that implements it on some API.
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef CONTROLLERENUMERATOR_H
#define CONTROLLERENUMERATOR_H

#include "controller.h"

class ControllerEnumerator : public QObject
{
Q_OBJECT
    public:
        ControllerEnumerator();
        virtual ~ControllerEnumerator();
        virtual QList<Controller*> queryDevices() = 0;
};

#endif