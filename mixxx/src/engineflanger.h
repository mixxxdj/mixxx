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

#include <qobject.h>

#include "engineobject.h"
#include "dlgflanger.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"

const int max_delay = 5000;  

class EngineFlanger : public EngineObject {
  Q_OBJECT
public:
  EngineFlanger(DlgFlanger *, const char *);
  ~EngineFlanger();
  CSAMPLE *process(const CSAMPLE *, const int);
  ControlPotmeter *potmeterDepth, *potmeterDelay, *potmeterLFOperiod;
  ControlPushButton *pushbuttonChannelA, *pushbuttonChannelB;
  valueType channel_A, channel_B;

public slots:
  void slotUpdateDepth(FLOAT_TYPE);
  void slotUpdateDelay(FLOAT_TYPE);
  void slotUpdateLFOperiod(FLOAT_TYPE);
  void slotUpdateChannelSelectA(valueType); 
  void slotUpdateChannelSelectB(valueType); 

private:
  CSAMPLE *process_buffer, *delay_buffer;
  FLOAT_TYPE depth; // the depth of the flanger [0..1];
  int  LFOamplitude;
  int average_delay_length;
  int LFOperiod; // Period of the LFO measured in samples
  int time;
  FLOAT_TYPE delay;
  int delay_pos;
  DlgFlanger *dlg;

};

#endif
