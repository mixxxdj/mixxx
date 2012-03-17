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

#ifndef CONTROLLOGPOTMETER_H
#define CONTROLLOGPOTMETER_H

#include "controlpotmeter.h"
#include "configobject.h"
#include <algorithm>

/**
 *@author Tue and Ken Haste Andersen
 */

class ControlLogpotmeter : public ControlPotmeter
{
    Q_OBJECT
public:
    ControlLogpotmeter(ConfigKey key, double dMaxValue=5.);
    
    double getValueFromWidget(double dValue);
    double getValueToWidget(double dValue);

    double GetMidiValue();
    
    void setValueFromMidi(MidiOpCode o, double v);

protected:

    // This is true, if the log potmeter is divided into two states, one from 0 to 1, and
    // the second from 1 to m_dMaxValue. Two states is often used with knobs where the first
    // half rotation is used to control a value between 0 and 1, and the second half between
    // 1 and some bigger value.
    bool m_bTwoState;
    
    double m_fB1, m_fB2;
};

#endif
