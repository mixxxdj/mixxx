/***************************************************************************
                          enginedelay.cpp  -  description
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

#include "enginedelay.h"
#include "controlengine.h"
#include "controlpotmeter.h"
#include "wknob.h"

/*----------------------------------------------------------------

  ----------------------------------------------------------------*/
EngineDelay::EngineDelay(const char *group)
{
    process_buffer = new CSAMPLE[MAX_BUFFER_LEN];
    delay_buffer = new CSAMPLE[max_delay];

    delay_pos = 0;

    ControlPotmeter *p = new ControlPotmeter(ConfigKey(group, "delay"), 0, max_delay);
}

EngineDelay::~EngineDelay()
{
    delete potmeter;
    delete [] process_buffer;
    delete [] delay_buffer;
}

CSAMPLE *EngineDelay::process(const CSAMPLE *source, const int buffer_size)
{
    int delay_source_pos = (delay_pos+max_delay-delay)%max_delay;
    if ((delay_source_pos<0) || (delay_source_pos>max_delay)) 
        qFatal("Error in EngineDelay: delay_source_pos = %d", delay_source_pos);

    for (int i=0; i<buffer_size; i++)
    {
        // put sample into delay buffer:
        delay_buffer[delay_pos] = source[i];
        delay_pos++;
        if (delay_pos >= max_delay)
            delay_pos=0;

        // Take "old" sample from delay buffer and mix it with the source buffer:
        process_buffer[i] = 0.5*(delay_buffer[delay_source_pos] + source[i]);
        delay_source_pos++;
        if (delay_source_pos >= max_delay)
            delay_source_pos=0;
    }
    return process_buffer;
}

