/***************************************************************************
                    mididevicehandler.h  -  MIDI devices thread
                             -------------------
    copyright            : (C) 2008 by Tom Care
    email                : psyc0de@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef MIDIDEVICEHANDLER_H
#define MIDIDEVICEHANDLER_H

#include "midiobject.h"

/*
 * This class abstracts all the horrible mess of OS specific MIDI handler
 * creation.
 */
class MidiDeviceHandler {
public:
	MidiDeviceHandler();
	~MidiDeviceHandler();
	MidiObject* getMidiPtr();
private:
	MidiObject* m_pMidi;
};

#endif
