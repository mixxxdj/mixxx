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
#include "portmidi.h"
/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectPortMidi : public MidiObject  {
public: 
    MidiObjectPortMidi(ConfigMIDI *c, QApplication *app);
    ~MidiObjectPortMidi();
protected:
    void run();

    PmEvent     buffer[2];
    PmStream    *midi;

};

#endif
