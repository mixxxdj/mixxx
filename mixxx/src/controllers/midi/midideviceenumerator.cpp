/***************************************************************************
                          MidiDeviceEnumerator.cpp
                       MIDI Device Enumerator Class
                       ----------------------------
    begin                : Fri Feb 26 2010
    copyright            : (C) 2010 Sean M. Pappalardo
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

#include "midideviceenumerator.h"

MidiDeviceEnumerator::MidiDeviceEnumerator() : QObject()
{
}

MidiDeviceEnumerator::~MidiDeviceEnumerator() {
    // In this function, the inheriting class must delete the Devices it creates
}