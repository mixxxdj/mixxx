/***************************************************************************
                          enginedelay.h  -  description
                             -------------------
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

#ifndef ENGINEDELAY_H
#define ENGINEDELAY_H

#include "engineobject.h"

class ControlEngine;
class WKnob;

const int max_delay = 20000; 

class EngineDelay : public EngineObject
{
public:
  EngineDelay(WKnob *, const char *group);
  ~EngineDelay();
    void notify(double) {};
  CSAMPLE *process(const CSAMPLE *, const int);
private:
  ControlEngine *potmeter;
  CSAMPLE *process_buffer, *delay_buffer;
  int delay, delay_pos;
};

#endif
