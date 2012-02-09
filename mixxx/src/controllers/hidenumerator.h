/**
* @file hidenumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief This class handles discovery and enumeration of DJ controllers that use the USB-HID protocol
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef HIDENUMERATOR_H
#define HIDENUMERATOR_H

#include "controllerenumerator.h"

class HidEnumerator : public ControllerEnumerator
{
    public:
        HidEnumerator();
        ~HidEnumerator();
        QList<Controller*> queryDevices();
    private:
        QList<Controller*> m_devices;
};

#endif