/***************************************************************************
                          controlbeat.cpp  -  description
                             -------------------
    begin                : Mon Apr 7 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
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

#include "controlbeat.h"

ControlBeat::ControlBeat(ConfigKey key) : ControlObject(key)
{
    value = -1.;
    time.start();
}

ControlBeat::~ControlBeat()
{
}

void ControlBeat::slotSetPosition(int pos)
{
    qDebug("ControlBeat: %i",pos);
    int elapsed = time.elapsed();
    time.restart();

    if (elapsed<=maxInterval)    
    {
        value = 6000./elapsed;
    }
    else
        value = 0.;

    emitValueChanged(value);
}

void ControlBeat::forceGUIUpdate()
{
}
    