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
    high = new EngineFilterIIR(bessel_highpass);
/*
    lowrbj = new EngineFilterRBJ();
    lowrbj->calc_filter_coeffs(6, 100., 44100., 0.3, 0., true);
    midrbj = new EngineFilterRBJ();
    midrbj->calc_filter_coeffs(6, 1000., 44100., 0.3, 0., true);
    highrbj = new EngineFilterRBJ();
    highrbj->calc_filter_coeffs(8, 10000., 48000., 0.3, 0., true);

    lowrbj = new EngineFilterRBJ();
    lowrbj->calc_filter_coeffs(0, 100., 48000., 0.3., 0., false);
    highrbj = new EngineFilterRBJ();
    highrbj->calc_filter_coeffs(1, 10000., 48000., 0.3., 0., false);
*/
    p = new ControlLogpotmeter(ConfigKey(group, "filterLow"), 3.);
    filterpotLow = new ControlEngine(p);

    p = new ControlLogpotmeter(ConfigKey(group, "filterMid"), 3.);
    filterpotMid = new ControlEngine(p);

    p = new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 3.);
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

//    CSAMPLE *p0 = lowrbj->process(source, buf_size);
//    CSAMPLE *p1 = highrbj->process(source, buf_size);

    
    for (int i=0; i<buf_size; i++)
        buffer[i] = filterpotLow->get()*p0[i] + filterpotHigh->get()*p1[i] + filterpotMid->get()*(source[i]-p0[i]-p1[i]);
    return buffer;
}
