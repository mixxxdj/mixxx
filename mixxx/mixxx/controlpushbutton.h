/***************************************************************************
                          controlpushbutton.h  -  description
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

#ifndef CONTROLPUSHBUTTON_H
#define CONTROLPUSHBUTTON_H

#include <controlobject.h>
#include "defs.h"
#include "midiobject.h"
#include <sys/timeb.h>
class MidiObject;
/**
  *@author Tue and Ken Haste Andersen
  */

enum positionType {up,down};
enum valueType {off,on};
enum buttonType {latching, momentaneous, simulated_latching};

class ControlPushButton : public ControlObject  {
 Q_OBJECT
protected:
  valueType value;       // the value (on or off)
  char* name;        // The name of the button
  buttonType kind;  // Determine whether the button is latching or not.
  valueType invert(valueType value);
  MidiObject *midi;
public:
  positionType position;  // position of the button (up or down)
  short int midino;
  short int midimask;
  ControlPushButton(char*, buttonType, int, int, MidiObject *);
  ~ControlPushButton();
  char* print();
  short int getmidino();
  valueType getValue();
  positionType getPosition();
  char* printValue();
  void setValue(valueType);
public slots:
  void slotSetPosition(positionType);
  void pressed();
  void released();
signals:
  void valueChanged(valueType);
};

#endif
