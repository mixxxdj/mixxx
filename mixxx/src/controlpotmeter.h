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

#include "configobject.h"
#include "controlobject.h"
#include "defs.h"
#include <algorithm>

/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPotmeter : public ControlObject
{
  Q_OBJECT
public:
    ControlPotmeter(ConfigKey key, FLOAT_TYPE=0.0, FLOAT_TYPE=1.0);
    ~ControlPotmeter();
    FLOAT_TYPE getValue();

    /** Returns the minimum allowed value */
    float getMin();
    /** Returns the maximum allowed value */
    float getMax();
    /** Sets the minimum and maximum allowed value. The control value is reset when calling
      * this method */
    void setRange(float fMin, float fMax);
    void setValue(int newpos);
    void setAccelUp(const QKeySequence) {};
    void setAccelDown(const QKeySequence) {};
public slots:
    void slotSetPositionExtern(float);
    void slotSetPositionMidi(MidiCategory c, int v);
protected:
    void forceGUIUpdate();
#define maxPosition 127
#define minPosition 0
#define middlePosition ((maxPosition-minPosition)/2)
#define positionrange (maxPosition-minPosition)
    FLOAT_TYPE maxvalue, minvalue, valuerange;
};

#endif
