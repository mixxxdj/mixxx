/***************************************************************************
                          enginevolume.cpp  -  description
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

#include "enginevolume.h"
#include "controllogpotmeter.h"
#include "controlengine.h"

/*----------------------------------------------------------------
  Volume effect.
  ----------------------------------------------------------------*/
EngineVolume::EngineVolume(ConfigKey key, double maxval)
{
    ControlLogpotmeter *p = new ControlLogpotmeter(key, maxval);
    potmeter = new ControlEngine(p);

    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineVolume::~EngineVolume()
{
    delete potmeter;
    delete buttonDown;
    delete buttonUp;
    delete [] buffer;
}

CSAMPLE *EngineVolume::process(const CSAMPLE *source, const int buffer_size)
{
    for (int i=0; i<buffer_size; i++)
        buffer[i] = source[i]*potmeter->get();
    return buffer;
}
