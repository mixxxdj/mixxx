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
   Purpose: Creates a new push-button. 
   Input:   key - Key for the configuration file
            kindtype - A button can be either be latching, momenteneous and simulated_latching.
	    _led - A led which is connected to the button.
   -------- ------------------------------------------------------ */
ControlPushButton::ControlPushButton(ConfigKey key, buttonType kindtype, WBulb* _led) : ControlObject(key)
{
    kind = kindtype;
    position = up;
    value = off;
    led = _led;
    if (led!=0)
	if (value==on)
	    led->setChecked(true);
	else
	    led->setChecked(false);
	
    //midimask = (int)pow(2,midibit);
};

ControlPushButton::~ControlPushButton()
{
    //ControlObject();
};

void ControlPushButton::slotSetPosition(int newpos)
{
    if (newpos == 0)
        slotSetPosition(up);
    else
        slotSetPosition(down);
}

/* -------- ------------------------------------------------------
   Purpose: Set the position of the button, and change the
            value correspondingly.
   Input:   The (new) position.
   Output:  The value is updated.
   -------- ------------------------------------------------------ */
void ControlPushButton::slotSetPosition(positionType newpos)
{
    switch (kind)
    {
    case latching:
        if (newpos==up)
            value = off;
        else
            value = on;
    case momentaneous:
        if (newpos==up)
            value = off;
        else
            value = on;
    case simulated_latching:
        if (newpos==down && position==up)
            value=invert(value);
    }
    position = newpos;

    // Control LED:
    if (led != 0)
	if (value==on)
	    led->setChecked(true);
	else
	    led->setChecked(false);

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

  // Control LED:
    if (led != 0)
	if (value==on)
	    led->setChecked(true);
	else
	    led->setChecked(false);
    
    emit valueChanged(value);
};

valueType ControlPushButton::invert(valueType value) {
    if (value==on)
        return off;
    else
        return on;
};
