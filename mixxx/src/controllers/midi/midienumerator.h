/**
* @file midienumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Tue 7 Feb 2012
* @brief This class handles discovery and enumeration of DJ controllers that use the MIDI protocol
*/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#ifndef MIDIENUMERATOR_H
#define MIDIENUMERATOR_H

#include "controllerenumerator.h"
#include "midicontroller.h"

class MidiEnumerator : public ControllerEnumerator
{
    public:
        MidiEnumerator();
        virtual ~MidiEnumerator();
        QList<Controller*> queryDevices();
//         virtual bool needPolling() { return false; };
    private:
        QList<MidiController*> m_devices;
};

#endif