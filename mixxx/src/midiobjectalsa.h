/***************************************************************************
                          midiobjectalsa.h  -  description
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

#ifndef MIDIOBJECTALSA_H
#define MIDIOBJECTALSA_H

#include <midiobject.h>
#include <sys/asoundlib.h>

/**
  *@author Tue & Ken Haste Andersen
  */

class MidiObjectALSA : public MidiObject  {
public: 
    MidiObjectALSA(ConfigObject *c);
    ~MidiObjectALSA();
protected:
    void run();

    snd_rawmidi_t   *handle;
    char            *buffer;
};

#endif
