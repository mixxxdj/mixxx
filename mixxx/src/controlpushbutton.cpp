/***************************************************************************
                          controlpushbutton.cpp  -  description
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

#include "controlpushbutton.h"

/* -------- ------------------------------------------------------
   Purpose: Creates a new push-button. Note that the button has
            to be registered separately by the midi controller
	    using a call to midi::addbutton.
   Input:   n - name
            kindtype - latching, momentaneous or simulated_latching
	    midino - number of the midi key.
	    midibit - the bit number of the button.
	    master - pointer to the control to which the button is
	            attached. This control is acknowledged when the
		    button is changed.
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(char* n, buttonType kindtype, 
				     int _midino, int midibit,
				     MidiObject* _midi) {
  name = n;
  kind = kindtype;
  position = up;
  value = off;
  midi = _midi;
  midino = _midino;
  midimask = (int)pow(2,midibit);
  midi->addbutton(this); // register the button at the midi controller.
};

ControlPushButton::~ControlPushButton() {
  midi->removebutton(this);
};

char* ControlPushButton::print(){
  return name;
};

short int ControlPushButton::getmidino() {
  return midino;
}
/* -------- ------------------------------------------------------
   Purpose: Set the position of the button, and change the
            value correspondingly.
   Input:   The (new) position.
   Output:  The value is updated.
   -------- ------------------------------------------------------ */
void ControlPushButton::slotSetPosition(positionType newpos){
  switch (kind) {
  case latching:
    if (newpos==up) value = off;
    else value = on;
  case momentaneous:
    if (newpos==up) value = off;
    else value = on;
  case simulated_latching:
    if (newpos==down && position==up)
      value=invert(value);
  }
  position = newpos;
  emit valueChanged(value);
};

void ControlPushButton::pressed() {
  slotSetPosition(down);
}

void ControlPushButton::released() {
  slotSetPosition(up);
}

valueType ControlPushButton::getValue() {
  return value;
};

positionType ControlPushButton::getPosition() {
  return position;
}

char* ControlPushButton::printValue() {
  if (value == on)
    return "on";
  else
    return "off";
}

void ControlPushButton::setValue(valueType newvalue){
  value = newvalue;
  emit valueChanged(value);
};

valueType ControlPushButton::invert(valueType value) {
  if (value==on) return off;
  else return on;
};
