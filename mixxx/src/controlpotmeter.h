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
#include <algorithm>

/**
  *@author Tue and Ken Haste Andersen
  */

class ControlPotmeter : public ControlObject
{
  Q_OBJECT
public:
    ControlPotmeter(ConfigKey key, double dMinValue=0.0, double dMaxValue=1.0);
    ~ControlPotmeter();
    /** Returns the minimum allowed value */
    double getMin();
    /** Returns the maximum allowed value */
    double getMax();
    /** Sets the minimum and maximum allowed value. The control value is reset when calling
      * this method */
    void setRange(double dMinValue, double dMaxValue);
    void setAccelUp(const QKeySequence) {};
    void setAccelDown(const QKeySequence) {};

public slots:
    void setValueFromWidget(double dValue);
    void setValueFromApp(double dValue);
    
protected:
    void updateWidget();
    void setValueFromMidi(MidiCategory c, int v);
    
    double m_dMaxValue, m_dMinValue, m_dValueRange;

#define maxPosition 127
#define minPosition 0
#define middlePosition ((maxPosition-minPosition)/2)
#define positionrange (maxPosition-minPosition)

};

#endif
