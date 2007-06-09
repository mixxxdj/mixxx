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

#include "controlpushbutton.h"
#include "controllogpotmeter.h"
#include "enginefilterblock.h"
#include "enginefilteriir.h"
#include "enginefilter.h"

EngineFilterBlock::EngineFilterBlock(const char *group)
{   
#ifdef __LOFI__
    low = new EngineFilterIIR(bessel_lowpass4_DJM800,4);
    band = new EngineFilterIIR(bessel_bandpass8_DJM800,8);
    high = new EngineFilterIIR(bessel_highpass4_DJM800,4);
    qDebug("Using LoFi EQs");
#else 
    low = new EngineFilter("LpBe4/70");
    band = new EngineFilter("BpBe4/70-13000");
    high = new EngineFilter("HpBe4/13000");

    //EngineFilter doesn't have any denormal handling so we add a slight amount of noise
    //don't worry this will all be filtered out long before it gets to the output, and is
    //far below the audible level
    int rand_state = 1;

    for(int i=0; i < SIZE_NOISE_BUF; i++){
        rand_state = rand_state * 1234567UL + 890123UL;
	rand_state = rand_state & 0x0FFFFFFFF;
	int    mantissa = rand_state & 0x807F0000; // Keep only most significant bits
        int   flt_rnd = mantissa | 0x1E000000;           // Set exponent
	whiteNoiseBuf[i] = *reinterpret_cast <const float *> (&flt_rnd);
    }
    
#endif
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
    
    filterpotLow = new ControlLogpotmeter(ConfigKey(group, "filterLow"), 4.);
    filterKillLow = new ControlPushButton(ConfigKey(group, "filterLowKill"),  true);

    filterpotMid = new ControlLogpotmeter(ConfigKey(group, "filterMid"), 4.);
    filterKillMid = new ControlPushButton(ConfigKey(group, "filterMidKill"),  true);

    filterpotHigh = new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 4.);
    filterKillHigh = new ControlPushButton(ConfigKey(group, "filterHighKill"),  true);
    
    m_pTemp1 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp2 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp3 = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineFilterBlock::~EngineFilterBlock()
{
    delete high;
    delete band;
    delete low;
    delete filterpotLow;
    delete filterKillLow;
    delete filterpotMid;
    delete filterKillMid;
    delete filterpotHigh;
    delete filterKillHigh;
}

void EngineFilterBlock::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE *pOutput = (CSAMPLE *)pOut;
    CSAMPLE fLow=0.f, fMid=0.f, fHigh=0.f;

    if (filterKillLow->get()==0.)
        fLow = filterpotLow->get(); //*0.7;
    if (filterKillMid->get()==0.)
        fMid = filterpotMid->get(); //*1.1;
    if (filterKillHigh->get()==0.)
        fHigh = filterpotHigh->get(); //*1.2;

    
/*    if (fLow == 1. && fMid == 1. && fHigh == 1.)
    {
        if (pIn!=pOut)
            memcpy(pOutput, pIn, sizeof(CSAMPLE) * iBufferSize);
    }
    else if (pIn!=pOut)
    {
        low->process(pIn, m_pTemp1, iBufferSize);
    band->process(pIn, m_pTemp2, iBufferSize);
        high->process(pIn, pOut, iBufferSize);
        
        for (int i=0; i<iBufferSize; ++i)
            pOutput[i] = 0.5*(fLow*m_pTemp1[i] + fHigh*pOut[i] + fMid*(pIn[i]-m_pTemp1[i]-pOut[i]);
    }
    else
*/

#ifndef __LOFI__
    //Add white noise to kill denormals
    //CSAMPLE *buf = (CSAMPLE *) pIn;
    //for(int i=0; i<iBufferSize; i++)
    //{
//	buf[i] = buf[i] + whiteNoiseBuf[ ++noiseCount % SIZE_NOISE_BUF ];
  //  }
#endif
    low->process(pIn, m_pTemp1, iBufferSize);
    band->process(pIn, m_pTemp2, iBufferSize);
    high->process(pIn, m_pTemp3, iBufferSize);
        
    for (int i=0; i<iBufferSize; ++i)
        pOutput[i] = (fLow*m_pTemp1[i] + fMid*m_pTemp2[i] + fHigh*m_pTemp3[i]);
}

