/***************************************************************************
                          portmidienumerator.h
                    PortMIDI Device Enumerator Class
                    --------------------------------
    begin                : Thu Feb 25 2010
    copyright            : (C) 2010 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

    This class handles discovery and enumeration of DJ controller devices
    that appear under the PortMIDI cross-platform API.

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef PORTMIDIENUMERATOR_H
#define PORTMIDIENUMERATOR_H

#include "midideviceenumerator.h"

class PortMidiEnumerator : public MidiDeviceEnumerator
{
    public:
        PortMidiEnumerator();
        ~PortMidiEnumerator();
        QList<MidiDevice*> queryDevices();
    private:
        QList<MidiDevice*> m_devices;
};

#endif