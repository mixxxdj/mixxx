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

    // Filter buffer
    buffer = new CSAMPLE[filterLength];
    for (int i=0; i<filterLength; i++)
        buffer[i] = 0.;
}

ControlBeat::~ControlBeat()
{
}

void ControlBeat::slotSetPosition(int pos)
{
    int elapsed = time.elapsed();
    time.restart();

    
    if (elapsed<=maxInterval)    
    {
        // Move back in filter one sample
        for (int i=1; i<filterLength; i++)
            buffer[i-1] = buffer[i];
            
        buffer[filterLength-1] = 1000.*(60./elapsed);
        if (buffer[filterLength-1]>maxBPM)
            buffer[filterLength-1] = maxBPM;
            
        value = 0.;
        for (int i=0; i<filterLength; i++)
            value += buffer[i];
        value /= filterLength;
    }
    else
        value = 0.;
    qDebug("ControlBeat: %f",value);

    emitValueChanged(value);
}

void ControlBeat::forceGUIUpdate()
{
}
    