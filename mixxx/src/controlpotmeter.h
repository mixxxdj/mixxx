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
  FLOAT_TYPE value;    // The actual value of the controller
#define maxPosition 127 
#define minPosition 0
#define middlePosition ((maxPosition-minPosition)/2)
#define positionrange (maxPosition-minPosition)
  FLOAT_TYPE maxvalue, minvalue, valuerange;
  MidiObject *midi;
 public:
  int midino;
  ControlPotmeter();
  ControlPotmeter(char* n, short int, MidiObject *, FLOAT_TYPE=0.0, FLOAT_TYPE=1.0);
  virtual ~ControlPotmeter();
  char* print();
  char getmidino();
  void setValue(FLOAT_TYPE newvalue);
  FLOAT_TYPE getValue();
  char getPosition();
  virtual void midiEvent(int);
public slots:
  virtual void slotSetPosition(int);
signals:
  void valueChanged(FLOAT_TYPE);
  void recievedMidi(int);
};

#endif
