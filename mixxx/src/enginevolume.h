/***************************************************************************
                          enginevolume.h  -  description
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

#ifndef ENGINEVOLUME_H
#define ENGINEVOLUME_H

#include "engineobject.h"
#include "configobject.h"

class ControlEngine;

class EngineVolume : public EngineObject {
public:
  EngineVolume(ConfigKey key, double maxval=1.);
  ~EngineVolume();
  CSAMPLE *process(const CSAMPLE*, const int);
    void notify(double) {};
private:
  CSAMPLE *buffer;
  ControlEngine *potmeterVolume;
};

#endif
