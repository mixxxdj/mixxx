/***************************************************************************
                          controlpotmeter.h  -  description
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

#ifndef CONTROLPOTMETER_H
#define CONTROLPOTMETER_H

#include "controlobject.h"
#include "defs.h"
#include "midiobject.h"
#include <algorithm>

class MidiObject;
/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPotmeter : public ControlObject  {
  Q_OBJECT
 protected:
  char position;  // position of the controller.
  char* name;        // The name of the controller
  FLOAT value;    // The actual value of the controller
  static char const maxPosition = 127;
  static char const minPosition  = 0;
  static char const middlePosition = (maxPosition-minPosition)/2;
  static char const positionrange = maxPosition-minPosition;
  FLOAT maxvalue, minvalue, valuerange;
  MidiObject *midi;
 public:
  int midino;
  ControlPotmeter();
  ControlPotmeter(char* n, short int, MidiObject *, FLOAT=0.0, FLOAT=1.0);
  virtual ~ControlPotmeter();
  char* print();
  char getmidino();
  void setValue(FLOAT newvalue);
  FLOAT getValue();
  char getPosition();
  virtual void midiEvent(int);
public slots:
  virtual void slotSetPosition(int);
signals:
  void valueChanged(FLOAT);
  void recievedMidi(int);
};

#endif
