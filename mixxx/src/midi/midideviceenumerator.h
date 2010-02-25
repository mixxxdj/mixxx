/***************************************************************************
                          MidiDeviceEnumerator.h
                       MIDI Device Enumerator Class
                       ----------------------------
    begin                : Thu Feb 25 2010
    copyright            : (C) 2010 Sean M. Pappalardo
    email                : spappalardo@mixxx.org

    This class handles discovery and enumeration of DJ controller devices.
    This is a base class that cannot be instantiated on its own:
    it must be inherited by a class that implements it on top of some API.
    (See PortMidiEnumarator, as of Feb 2010.)

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef MIDIDEVICEENUMERATOR_H
#define MIDIDEVICEENUMERATOR_H

#include "mididevice.h"

class MidiDeviceEnumerator : public QObject
{
Q_OBJECT
    public:
        MidiDeviceEnumerator() { };
        virtual ~MidiDeviceEnumerator() = 0;
        virtual QList<MidiDevice*> queryDevices() = 0;
};

#endif