/***************************************************************************
                          midiobjectportmidi.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
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

#ifndef MIDIOBJECTPORTMIDI_H
#define MIDIOBJECTPORTMIDI_H

#include <midiobject.h>
#include <portmidi.h>
#include <porttime.h>

#include <stdlib.h>
#include <unistd.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectPortMidi : public MidiObject
{
public: 
    MidiObjectPortMidi(ConfigObject<ConfigValueMidi> *, QApplication *, QString device);
    ~MidiObjectPortMidi();
    void devOpen(QString device);
    void devClose();
protected:
    void run();

    PmEvent     buffer[10];
    PmStream    *midi;

};

#endif
