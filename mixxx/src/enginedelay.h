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

#include <qobject.h>

#include "engineobject.h"
#include "controlpotmeter.h"
#include "wknob.h"

const int max_delay = 20000; 

class EngineDelay : public EngineObject {
  Q_OBJECT
public:
  EngineDelay(WKnob *);
  ~EngineDelay();
  CSAMPLE *process(const CSAMPLE *, const int);
  ControlPotmeter *potmeter;

public slots:
  void slotUpdate(FLOAT_TYPE);

private:
  CSAMPLE *process_buffer, *delay_buffer;
  int  delay;
  int delay_pos;

};

#endif
