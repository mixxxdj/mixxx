/***************************************************************************
                          controlnull.h  -  description
                             -------------------
    begin                : Sat Jun 15 2002
    copyright            : (C) 2002 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CONTROLNULL_H
#define CONTROLNULL_H

#include "controlobject.h"

/**
  *@author Tue & Ken Haste Andersen
  */

class ControlNull : public ControlObject  {
    Q_OBJECT
public:
    ControlNull();
    ~ControlNull();
};

#endif
