/***************************************************************************
                          engineflanger.h  -  description
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

#ifndef ENGINEFLANGER_H
#define ENGINEFLANGER_H

#include "engineobject.h"

class DlgFlanger;
class ControlEngine;

const int max_delay = 5000;  

class EngineFlanger : public EngineObject {
public:
  EngineFlanger(DlgFlanger *dlg_flanger, const char *group);
  ~EngineFlanger();
    void notify(double) {};
  CSAMPLE *process(const CSAMPLE *, const int);
  ControlEngine *getButtonA();
  ControlEngine *getButtonB();
private:
  ControlEngine *potmeterDepth, *potmeterDelay, *potmeterLFOperiod;
  ControlEngine *pushbuttonChannelA, *pushbuttonChannelB;
  CSAMPLE *process_buffer, *delay_buffer;
  int  LFOamplitude;
  int average_delay_length;
  int time;
  FLOAT_TYPE delay;
  int delay_pos;
  DlgFlanger *dlg;
};

#endif
