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

/*----------------------------------------------------------------
  Volume effect.
  ----------------------------------------------------------------*/
EngineVolume::EngineVolume(const ConfigKey key)
{
    potmeter = new ControlLogpotmeter(key, 5.0);
    volume = 1.0;
    buffer = new CSAMPLE[MAX_BUFFER_LEN];

    connect(potmeter, SIGNAL(valueChanged(FLOAT_TYPE)), this, SLOT(slotUpdate(FLOAT_TYPE)));
}

EngineVolume::~EngineVolume()
{
    delete potmeter;
    delete [] buffer;
}

void EngineVolume::slotUpdate(FLOAT_TYPE newvalue)
{
    volume = newvalue;
    //qDebug("Volume: %f",volume);
}

CSAMPLE *EngineVolume::process(const CSAMPLE *source, const int buffer_size)
{
    for (int i=0; i<buffer_size; i++)
        buffer[i] = source[i]*volume;
    return buffer;
}
