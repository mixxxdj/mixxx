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

#include "controlobject.h"
#include "defs.h"
#include "wbulb.h"
#include <sys/timeb.h>

/**
  *@author Tue and Ken Haste Andersen
  */

enum positionType {up,down};
enum valueType {off,on};
enum buttonType {latching, momentaneous, simulated_latching};

class ControlPushButton : public ControlObject
{
 Q_OBJECT
public:
  ControlPushButton(ConfigKey key, buttonType, WBulb *led = 0);
  ~ControlPushButton();
  char* print();
  valueType getValue();
  positionType getPosition();
  char* printValue();
  void setValue(valueType);

  positionType position;  // position of the button (up or down)
  //short int midimask;
public slots:
  void slotSetPosition(int);
  void slotSetPosition(positionType);
  void pressed();
  void released();
signals:
  void valueChanged(valueType);
protected:
  valueType value;       // the value (on or off)
  char* name;        // The name of the button
  buttonType kind;  // Determine whether the button is latching or not.
  valueType invert(valueType value);
  MidiObject *midi;
  WBulb *led;
};

#endif
