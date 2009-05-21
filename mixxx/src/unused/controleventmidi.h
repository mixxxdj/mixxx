/***************************************************************************
                          controleventmidi.h  -  description
                             -------------------
    begin                : Thu Feb 20 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#ifndef CONTROLEVENTMIDI_H
#define CONTROLEVENTMIDI_H

#include <qevent.h>
#include "midiobject.h"

/**
  *@author Tue & Ken Haste Andersen
  *
  * Event used in communication from MidiObject to ControlObject
  *
  */

class ControlEventMidi : public QEvent {
public: 
    ControlEventMidi(MidiCategory category, char channel, char control, char value);
    ~ControlEventMidi();
    MidiCategory category() const;
    char channel() const;
    char control() const;
    char value() const;
private:
    MidiCategory mcategory;
    char mchannel, mcontrol, mvalue;
};

#endif
