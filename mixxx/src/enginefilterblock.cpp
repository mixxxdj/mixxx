/***************************************************************************
                          enginefilterblock.cpp  -  description
                             -------------------
    begin                : Thu Apr 4 2002
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

#include "enginefilterblock.h"
#include "controllogpotmeter.h"
#include "controlengine.h"

EngineFilterBlock::EngineFilterBlock(const char *group)
{
    ControlLogpotmeter *p;

    low = new EngineFilterIIR(bessel_lowpass);
    p = new ControlLogpotmeter(ConfigKey(group, "filterLow"), 5.);
    filterpotLow = new ControlEngine(p);

    p = new ControlLogpotmeter(ConfigKey(group, "filterMid"), 5.);
    filterpotMid = new ControlEngine(p);

    high = new EngineFilterIIR(bessel_highpass);
    p = new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 5.);
    filterpotHigh = new ControlEngine(p);

    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterBlock::~EngineFilterBlock()
{
    delete [] buffer;
    delete high;
    delete low;
}

CSAMPLE *EngineFilterBlock::process(const CSAMPLE *source, const int buf_size)
{
    CSAMPLE *p0 = low->process(source,buf_size);
    CSAMPLE *p1 = high->process(source,buf_size);
    
    for (int i=0; i<buf_size; i++)
        buffer[i] = filterpotLow->get()*p0[i] + filterpotHigh->get()*p1[i] + filterpotMid->get()*(source[i]-p0[i]-p1[i]);
    
    return buffer;
}
