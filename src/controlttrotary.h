/***************************************************************************
                          controlttrotary.h  -  description
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

#ifndef CONTROLTTROTARY_H
#define CONTROLTTROTARY_H

#include "configobject.h"
#include "controlobject.h"

class ControlTTRotary : public ControlObject {
    Q_OBJECT
  public:
    ControlTTRotary(ConfigKey key);
};

#endif
