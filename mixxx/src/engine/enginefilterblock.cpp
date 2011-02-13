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

#include <QtDebug>
#include "controlpushbutton.h"
#include "controllogpotmeter.h"
#include "enginefilterblock.h"
#include "enginefilteriir.h"
#include "enginefilter.h"
#include "enginefilterbutterworth8.h"
#include "sampleutil.h"

EngineFilterBlock::EngineFilterBlock(const char * group)
{
    ilowFreq = 0;
    ihighFreq = 0;
    blofi = false;

#ifdef __LOFI__
    low = new EngineFilterIIR(bessel_lowpass4_DJM800,4);
    band = new EngineFilterIIR(bessel_bandpass8_DJM800,8);
    high = new EngineFilterIIR(bessel_highpass4_DJM800,4);
    qDebug() << "Using LoFi EQs";
#else

    //Setup Filter Controls
    if(ControlObject::getControl(ConfigKey("[Mixer Profile]", "LoEQFrequency")) == NULL)
    {
        m_loEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);
        m_hiEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);
        m_lofiEq = new ControlPushButton(ConfigKey("[Mixer Profile]", "LoFiEQs"));
    }
    else
    {
        m_loEqFreq = (ControlPotmeter*) ControlObject::getControl(ConfigKey("[Mixer Profile]", "LoEQFrequency"));
        m_hiEqFreq = (ControlPotmeter*) ControlObject::getControl(ConfigKey("[Mixer Profile]", "HiEQFrequency"));
        m_lofiEq = (ControlPushButton*) ControlObject::getControl(ConfigKey("[Mixer Profile]", "LoFiEQs"));
    }

    high = band = low = NULL;

    //Load Defaults
    setFilters(true);

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
    filterKillLow = new ControlPushButton(ConfigKey(group, "filterLowKill"));
    filterKillLow->setToggleButton(true);

    filterpotMid = new ControlLogpotmeter(ConfigKey(group, "filterMid"), 4.);
    filterKillMid = new ControlPushButton(ConfigKey(group, "filterMidKill"));
    filterKillMid->setToggleButton(true);

    filterpotHigh = new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 4.);
    filterKillHigh = new ControlPushButton(ConfigKey(group, "filterHighKill"));
    filterKillHigh->setToggleButton(true);

    m_pTemp1 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp2 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp3 = new CSAMPLE[MAX_BUFFER_LEN];

    memset(m_pTemp1, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pTemp2, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pTemp3, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
}

EngineFilterBlock::~EngineFilterBlock()
{
    delete high;
    delete band;
    delete low;
    delete [] m_pTemp3;
    delete [] m_pTemp2;
    delete [] m_pTemp1;
    delete filterpotLow;
    delete filterKillLow;
    delete filterpotMid;
    delete filterKillMid;
    delete filterpotHigh;
    delete filterKillHigh;
}

void EngineFilterBlock::setFilters(bool forceSetting)
{
    if((ilowFreq != m_loEqFreq->get()) || (ihighFreq != m_hiEqFreq->get()) || (blofi != m_lofiEq->get()) || forceSetting)
    {
        delete low;
        delete band;
        delete high;
        ilowFreq = m_loEqFreq->get();
        ihighFreq = m_hiEqFreq->get();
        blofi = m_lofiEq->get();
        if(blofi)
        {
            // why is this DJM800 at line ~34 (LOFI ifdef) and just
            // bessel_lowpass# here? bkgood
            low = new EngineFilterIIR(bessel_lowpass4,4);
            band = new EngineFilterIIR(bessel_bandpass,8);
            high = new EngineFilterIIR(bessel_highpass4,4);
        }
        else
        {
            low = new EngineFilterButterworth8(FILTER_LOWPASS, 44100, m_loEqFreq->get());
            band = new EngineFilterButterworth8(FILTER_BANDPASS, 44100, m_loEqFreq->get(), m_hiEqFreq->get());
            high = new EngineFilterButterworth8(FILTER_HIGHPASS, 44100, m_hiEqFreq->get());
        }

    }
}

void EngineFilterBlock::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    float fLow=0.f, fMid=0.f, fHigh=0.f;


    if (filterKillLow->get()==0.)
        fLow = filterpotLow->get(); //*0.7;
    if (filterKillMid->get()==0.)
        fMid = filterpotMid->get(); //*1.1;
    if (filterKillHigh->get()==0.)
        fHigh = filterpotHigh->get(); //*1.2;

#ifndef __LOFI__
    setFilters();
#endif

    low->process(pIn, m_pTemp1, iBufferSize);
    band->process(pIn, m_pTemp2, iBufferSize);
    high->process(pIn, m_pTemp3, iBufferSize);

    SampleUtil::copy3WithGain(pOutput,
                              m_pTemp1, fLow,
                              m_pTemp2, fMid,
                              m_pTemp3, fHigh, iBufferSize);
}

