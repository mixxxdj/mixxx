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
	    is 0, for midibyte=64 the value is 1 and the upper 
	    value is set by maxvalue.
   Input:   n - name
	    midino - number of the midi controller.
	    midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlLogpotmeter::ControlLogpotmeter(ConfigKey *_key, FLOAT_TYPE _maxvalue) : ControlPotmeter(_key)
{
    a = 1;
    b = log10(2)/middlePosition;
    b2 = log10((_maxvalue+1)/2)/middlePosition;
    a2 = 2*pow(10, -middlePosition*b);
    //qDebug("%g %g",a,b);
}

/* -------- ------------------------------------------------------
   Purpose: Set the position of the potmeter, and change the
            value correspondingly.
   Input:   The (new) position.
   Output:  The value is updated.
   -------- ------------------------------------------------------ */
#define min(a,b)            (((a) < (b)) ? (a) : (b))
void ControlLogpotmeter::slotSetPosition(int _newpos)
{
  char newpos =(char)_newpos;

  // Ensure that the position is within bounds:
  position = max(minPosition, min(newpos, maxPosition));
  // Calculate the value linearly:
  if (newpos <= middlePosition)
      value = a*pow(10, b*(FLOAT_TYPE)newpos) - 1;
  else
      value = a2*pow(10, b2*(FLOAT_TYPE)newpos) - 2;

  //qDebug("Logpotmeter, midi:%i value:%g", _newpos, value);

  emit valueChanged(value);
}


