/***************************************************************************
                          hss1394enumerator.h
                    HSS1394 Device Enumerator Class
                    --------------------------------
    begin                : Fri Feb 26 2010
    copyright            : (C) 2010 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

    This class handles discovery and enumeration of DJ controller devices
    that use the HSS1394 protocol and appear under the HSS1394
    cross-platform API.

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef HSS1394ENUMERATOR_H
#define HSS1394ENUMERATOR_H

#include "midideviceenumerator.h"

class Hss1394Enumerator : public MidiDeviceEnumerator
{
    public:
        Hss1394Enumerator();
        ~Hss1394Enumerator();
        QList<MidiDevice*> queryDevices();
    private:
        QList<MidiDevice*> m_devices;
};

#endif