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

#include "controlpotmeter.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new potmeter
   Input:   n - name
	    midino - number of the midi controller.
	    master - pointer to the control to which the potmeter is
	            attached. This control is acknowledged when the
		    potmeter is changed.
	    midicontroller - pointer to the midi controller.
   -------- ------------------------------------------------------ */
ControlPotmeter::ControlPotmeter() {}

ControlPotmeter::ControlPotmeter(char* n, short int _midino, MidiObject *_midi,
				 FLOAT _minvalue=0.0, FLOAT _maxvalue=1.0)
{
  name = n;
  position = middlePosition;
  minvalue = _minvalue;
  maxvalue = _maxvalue;
  valuerange = maxvalue-minvalue;
  value = minvalue + 0.5*(maxvalue-minvalue);

  midi = _midi;
  midino = _midino;
  midi->addpotmeter(this);
}

ControlPotmeter::~ControlPotmeter() {
  midi->removepotmeter(this);
}

char* ControlPotmeter::print() {
  return name;
}

/* -------- ------------------------------------------------------
   Purpose: Set the position of the potmeter, and change the
            value correspondingly.
   Input:   The (new) position.
   Output:  The value is updated.
   -------- ------------------------------------------------------ */
void ControlPotmeter::slotSetPosition(int _newpos) {
  char newpos =(char)_newpos;
  char static const maxPosition = 127;
  char static const minPosition  = 0;

  // Ensure that the position is within bounds:
  position = std::max(minPosition,std::min(newpos, maxPosition));
  // Calculate the value linearly:
  value = (valuerange/positionrange)*
    ((maxPosition-newpos)-minPosition)+minvalue;
  qDebug("Controlpotmeter: changed %s to %g.",name,value);

  emit valueChanged(value);
}

char ControlPotmeter::getPosition()
{
  return position;
}

void ControlPotmeter::setValue(FLOAT newvalue)
{
  value = newvalue;
  emit valueChanged(value);
}

FLOAT ControlPotmeter::getValue()
{
  return value;
}

char ControlPotmeter::getmidino()
{ return midino; }
