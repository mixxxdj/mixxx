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
#include "controlobjectslave.h"
#include "engine/enginefilterblock.h"
#include "engine/enginefilteriir.h"
#include "engine/enginefilter.h"
#include "engine/enginefilterbutterworth8.h"
#include "sampleutil.h"
#include "util/timer.h"

ControlPotmeter* EngineFilterBlock::s_loEqFreq = NULL;
ControlPotmeter* EngineFilterBlock::s_hiEqFreq = NULL;
ControlPushButton* EngineFilterBlock::s_lofiEq = NULL;
ControlPushButton* EngineFilterBlock::s_EnableEq = NULL;

EngineFilterBlock::EngineFilterBlock(const char* group)
{
    ilowFreq = 0;
    ihighFreq = 0;
    blofi = false;
    m_eqNeverTouched = true;

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");
    m_iOldSampleRate = 0;

    //Setup Filter Controls

    if (s_loEqFreq == NULL) {
        s_loEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);
        s_hiEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);
        s_lofiEq = new ControlPushButton(ConfigKey("[Mixer Profile]", "LoFiEQs"));
        s_EnableEq = new ControlPushButton(ConfigKey("[Mixer Profile]", "EnableEQs"));
    }

    high = band = low = NULL;

    //Load Defaults
    setFilters(true);

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
    filterKillLow->setButtonMode(ControlPushButton::POWERWINDOW);

    filterpotMid = new ControlLogpotmeter(ConfigKey(group, "filterMid"), 4.);
    filterKillMid = new ControlPushButton(ConfigKey(group, "filterMidKill"));
    filterKillMid->setButtonMode(ControlPushButton::POWERWINDOW);

    filterpotHigh = new ControlLogpotmeter(ConfigKey(group, "filterHigh"), 4.);
    filterKillHigh = new ControlPushButton(ConfigKey(group, "filterHighKill"));
    filterKillHigh->setButtonMode(ControlPushButton::POWERWINDOW);

    m_pTemp1 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp2 = new CSAMPLE[MAX_BUFFER_LEN];
    m_pTemp3 = new CSAMPLE[MAX_BUFFER_LEN];

    memset(m_pTemp1, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pTemp2, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);
    memset(m_pTemp3, 0, sizeof(CSAMPLE) * MAX_BUFFER_LEN);

    old_low = old_mid = old_high = 1.0;
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
    delete m_pSampleRate;

    // Delete and clear these static controls. We need to clear them so that
    // other instances of EngineFilterBlock won't delete them as well.
    delete s_loEqFreq;
    s_loEqFreq = NULL;
    delete s_hiEqFreq;
    s_hiEqFreq = NULL;
    delete s_lofiEq;
    s_lofiEq = NULL;
    delete s_EnableEq;
    s_EnableEq = NULL;
}

void EngineFilterBlock::setFilters(bool forceSetting) {
    int iSampleRate = static_cast<int>(m_pSampleRate->get());
    if (m_iOldSampleRate != iSampleRate ||
            (ilowFreq != (int)s_loEqFreq->get()) ||
            (ihighFreq != (int)s_hiEqFreq->get()) ||
            (blofi != (int)s_lofiEq->get()) ||
            forceSetting) {
        delete low;
        delete band;
        delete high;
        ilowFreq = (int)s_loEqFreq->get();
        ihighFreq = (int)s_hiEqFreq->get();
        blofi = (int)s_lofiEq->get();
        m_iOldSampleRate = iSampleRate;
        if (blofi) {
            // why is this DJM800 at line ~34 (LOFI ifdef) and just
            // bessel_lowpass# here? bkgood
            low = new EngineFilterIIR(bessel_lowpass4,4);
            band = new EngineFilterIIR(bessel_bandpass,8);
            high = new EngineFilterIIR(bessel_highpass4,4);
        } else {
            low = new EngineFilterButterworth8Low(iSampleRate, ilowFreq);
            band = new EngineFilterButterworth8Band(iSampleRate, ilowFreq, ihighFreq);
            high = new EngineFilterButterworth8High(iSampleRate, ihighFreq);
        }

    }
}

void EngineFilterBlock::process(CSAMPLE* pInOut, const int iBufferSize) {
    ScopedTimer t("EngineFilterBlock::process");

    // Check if EQ processing is disabled.
    if (!s_EnableEq->get()) {
        return;
    }

    float fLow=0.f, fMid=0.f, fHigh=0.f;
    if (filterKillLow->get()==0.)
        fLow = filterpotLow->get(); //*0.7;
    if (filterKillMid->get()==0.)
        fMid = filterpotMid->get(); //*1.1;
    if (filterKillHigh->get()==0.)
        fHigh = filterpotHigh->get(); //*1.2;

    setFilters();

    // If the user has never touched the Eq controls (they are still at zero)
    // Then pass through.  As soon as the user twiddles one, actually activate
    // the EQ code and crossfade to it.  This will save CPU if the user never
    // uses EQ but also doesn't know to disable it.
    if (m_eqNeverTouched) {
        if (fLow == 1. && fMid == 1. && fHigh == 1.) {
            return;
        }
        m_eqNeverTouched = false;
    }

    low->process(pInOut, m_pTemp1, iBufferSize);
    band->process(pInOut, m_pTemp2, iBufferSize);
    high->process(pInOut, m_pTemp3, iBufferSize);

    if (fLow != old_low || fMid != old_mid || fHigh != old_high) {
        SampleUtil::copy3WithRampingGain(pInOut,
                                         m_pTemp1, old_low, fLow,
                                         m_pTemp2, old_mid, fMid,
                                         m_pTemp3, old_high, fHigh,
                                         iBufferSize);
    } else {
        SampleUtil::copy3WithGain(pInOut,
                          m_pTemp1, fLow,
                          m_pTemp2, fMid,
                          m_pTemp3, fHigh, iBufferSize);
    }

    old_low = fLow;
    old_mid = fMid;
    old_high = fHigh;
}
