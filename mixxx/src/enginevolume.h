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

class ControlEngine;
class ConfigKey;

class EngineVolume : public EngineObject {
public:
    EngineVolume(ConfigKey key, double maxval=1.);
    ~EngineVolume();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

private:
    CSAMPLE *buffer;
    ControlEngine *potmeter;
};

#endif
