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
#include "controlpushbutton.h"
#include "controlengine.h"

EngineFilterBlock::EngineFilterBlock(const char *group)
{
    ControlLogpotmeter *p;
    ControlPushButton *pb;

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
    p = new ControlLogpotmeter(ConfigKey(group, "filterLow"), 4.);
    filterpotLow = new ControlEngine(p);
    pb = new ControlPushButton(ConfigKey(group, "filterLowKill"),  true);
    filterKillLow = new ControlEngine(pb);

    p = new ControlLogpotmeter(ConfigKey(group, "filterMid"), 4.);
    filterpotMid = new ControlEngine(p);
    pb = new ControlPushButton(ConfigKey(group, "filterMidKill"),  true);
    filterKillMid = new ControlEngine(pb);

    p = new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 4.);
    filterpotHigh = new ControlEngine(p);
    pb = new ControlPushButton(ConfigKey(group, "filterHighKill"),  true);
    filterKillHigh = new ControlEngine(pb);

    buffer = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterBlock::~EngineFilterBlock()
{
    delete [] buffer;
    delete high;
    delete low;
    delete filterpotLow;
    delete filterKillLow;
    delete filterpotMid;
    delete filterKillMid;
    delete filterpotHigh;
    delete filterKillHigh;
}

CSAMPLE *EngineFilterBlock::process(const CSAMPLE *source, const int buf_size)
{
    CSAMPLE fLow=0.f, fMid=0.f, fHigh=0.f;

    if (filterKillLow->get()==0.)
        fLow = filterpotLow->get();
    if (filterKillMid->get()==0.)
        fMid = filterpotMid->get();
    if (filterKillHigh->get()==0.)
        fHigh = filterpotHigh->get();

    if ((fLow == 1.) && (fMid == 1.) && (fHigh == 1.))
    {
        memcpy(buffer, source, sizeof(CSAMPLE) * buf_size);
        return buffer;
    }

    CSAMPLE *p0 = low->process(source,buf_size);
    CSAMPLE *p1 = high->process(source,buf_size);

    for (int i=0; i<buf_size; i++)
        buffer[i] = fLow*p0[i] + fHigh*p1[i] + fMid*(source[i]-p0[i]-p1[i]);
    return buffer;
}
