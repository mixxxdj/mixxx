/***************************************************************************
                          controlpotmeter.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "controllogpotmeter.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new logarithmic potmeter, where the value is
            given by: value = a*10^(b*midibyte) - 1. The lower value
	    is set by _minvalue, and for midibyte=64, the value
	    is 1.
   Input:   n - name
	    midino - number of the midi controller.
	    midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlLogpotmeter::ControlLogpotmeter(char* n, int _midino, MidiObject *_midi,
				       FLOAT _minvalue=-0.9) : ControlPotmeter(n,_midino,_midi) {
  a = _minvalue+1;
  b = -2*log10(a)/(FLOAT)maxPosition;
  qDebug("%g %g",a,b);
}

/* -------- ------------------------------------------------------
   Purpose: Set the position of the potmeter, and change the
            value correspondingly.
   Input:   The (new) position.
   Output:  The value is updated.
   -------- ------------------------------------------------------ */
void ControlLogpotmeter::slotSetPosition(int _newpos)
{
  char newpos =(char)_newpos;

  char static const maxPosition = 127;
  char static const minPosition  = 0;

  // Ensure that the position is within bounds:
  position = std::max(minPosition,std::min(newpos, maxPosition));
  // Calculate the value linearly:
  value = a*pow(10, b*(FLOAT)newpos) - 1;
  qDebug("Logpotmeter, midi:%i value:%g", _newpos, value);

  emit valueChanged(value);
}
