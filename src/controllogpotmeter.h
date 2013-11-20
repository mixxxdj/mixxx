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

class ControlLogpotmeter : public ControlPotmeter {
    Q_OBJECT
  public:
    ControlLogpotmeter(ConfigKey key, double dMaxValue=5.);
};

#endif
