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

class ControlPushButton;

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
    /** Sets the step size of the associated PushButtons */
    void setStep(double);
    /** Sets the small step size of the associated PushButtons */
    void setSmallStep(double);
    /** Sets the minimum and maximum allowed value. The control value is reset when calling
      * this method */
    void setRange(double dMinValue, double dMaxValue);
    double getValueFromWidget(double dValue);
    double getValueToWidget(double dValue);
    double GetMidiValue();

public slots:
    void setValueFromThread(double dValue);
    void setValueFromEngine(double dValue);
    /** Increases the value. This method is called from an associated PushButton control */
    void incValue(double);
    /** Decreases the value. This method is called from an associated PushButton control */
    void decValue(double);
    /** Increases the value by smaller step. */
    void incSmallValue(double);
    /** Decreases the value by smaller step. */
    void decSmallValue(double);
    /** Sets the value to 1.0. */
    void setToOne(double);
    /** Sets the value to -1.0. */
    void setToMinusOne(double);
    /** Sets the value to 0.0. */
    void setToZero(double);
    // Sets the control to its default
    void setToDefault(double);
    /** Toggles the value between 0.0 and 1.0. */
    void toggleValue(double);
    /** Toggles the value between -1.0 and 0.0. */
    void toggleMinusValue(double);

protected:
    void setValueFromMidi(MidiOpCode o, double v);

    double m_dMaxValue, m_dMinValue, m_dValueRange, m_dStep, m_dSmallStep;
};

#endif
