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
#include "engine/enginefilterbessel4.h"
#include "engine/enginefilter.h"
#include "engine/enginefilterbutterworth8.h"
#include "sampleutil.h"
#include "util/timer.h"
#include "util/defs.h"

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
    m_iOldSampleRate = static_cast<int>(m_pSampleRate->get());

    // Setup Filter Controls

    if (s_loEqFreq == NULL) {
        s_loEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);
        s_hiEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);
        s_lofiEq = new ControlPushButton(ConfigKey("[Mixer Profile]", "LoFiEQs"));
        s_EnableEq = new ControlPushButton(ConfigKey("[Mixer Profile]", "EnableEQs"));
    }

    // Load Defaults
    lowLight = new EngineFilterBessel4Low(m_iOldSampleRate, 246);
    bandLight = new EngineFilterBessel4Band(m_iOldSampleRate, 246, 2484);
    highLight = new EngineFilterBessel4High(m_iOldSampleRate, 2484);
    lowDef = new EngineFilterButterworth8Low(m_iOldSampleRate, 246);
    bandDef = new EngineFilterButterworth8Band(m_iOldSampleRate, 246, 2484);
    highDef = new EngineFilterButterworth8High(m_iOldSampleRate, 2484);

    low = lowDef;
    band = bandDef;
    high = highDef;

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

    m_pLowBuf = new CSAMPLE[MAX_BUFFER_LEN];
    m_pBandBuf = new CSAMPLE[MAX_BUFFER_LEN];
    m_pHighBuf = new CSAMPLE[MAX_BUFFER_LEN];

    old_low = old_mid = old_high = 1.0;
}

EngineFilterBlock::~EngineFilterBlock()
{
    delete lowLight;
    delete bandLight;
    delete highLight;
    delete lowDef;
    delete bandDef;
    delete highDef;
    delete [] m_pHighBuf;
    delete [] m_pBandBuf;
    delete [] m_pLowBuf;
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

void EngineFilterBlock::setFilters() {
    int iSampleRate = static_cast<int>(m_pSampleRate->get());
    if (m_iOldSampleRate != iSampleRate ||
            (ilowFreq != (int)s_loEqFreq->get()) ||
            (ihighFreq != (int)s_hiEqFreq->get()) ||
            (blofi != (int)s_lofiEq->get())) {
        ilowFreq = (int)s_loEqFreq->get();
        ihighFreq = (int)s_hiEqFreq->get();
        blofi = (int)s_lofiEq->get();
        m_iOldSampleRate = iSampleRate;
        if (blofi) {
            lowLight->setFrequencyCorners(iSampleRate, ilowFreq);
            bandLight->setFrequencyCorners(iSampleRate, ilowFreq, ihighFreq);
            highLight->setFrequencyCorners(iSampleRate, ihighFreq);
            low = lowLight;
            band = bandLight;
            high = highLight;
        } else {
            lowDef->setFrequencyCorners(iSampleRate, ilowFreq);
            bandDef->setFrequencyCorners(iSampleRate, ilowFreq, ihighFreq);
            highDef->setFrequencyCorners(iSampleRate, ihighFreq);
            low = lowDef;
            band = bandDef;
            high = highDef;
        }
    }
}

void EngineFilterBlock::process(CSAMPLE* pInOut, const int iBufferSize) {
    ScopedTimer t("EngineFilterBlock::process");

    // Check if EQ processing is disabled.
    if (!s_EnableEq->get()) {
        return;
    }

    float fLow = 0.f, fMid = 0.f, fHigh = 0.f;
    if (filterKillLow->get() == 0.) {
        fLow = filterpotLow->get();
    }
    if (filterKillMid->get() == 0.) {
        fMid = filterpotMid->get();
    }
    if (filterKillHigh->get() == 0.) {
        fHigh = filterpotHigh->get();
    }

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

    float fDry = 0;
    // This is the RGBW Mix. It is currently not working,
    // because of the group delay, introduced by the filters.
    // Since the dry signal has no delay, we get a frequency distorsion
    // once it is mixed together with the filtered signal
    // This might be fixed later by an allpass filter for the dry signal
    // or zero-phase no-lag filters
    // "Linear Phase EQ" "filtfilt()"
    //fDry = qMin(qMin(fLow, fMid), fHigh);
    //fLow -= fDry;
    //fMid -= fDry;
    //fHigh -= fDry;

    setFilters();

    // Process the new EQ'd signals.
    // They use up to 16 frames history so in case we are just starting,
    // 16 frames are junk, this is handled by ramp_delay
    int ramp_delay = 0;
    if (fLow || old_low) {
        low->process(pInOut, m_pLowBuf, iBufferSize);
        if(old_low == 0) {
            ramp_delay = 30;
        }
    }
    if (fMid || old_mid) {
        band->process(pInOut, m_pBandBuf, iBufferSize);
        if(old_mid== 0) {
            ramp_delay = 30;
        }
    }
    if (fHigh || old_high) {
        high->process(pInOut, m_pHighBuf, iBufferSize);
        if(old_high == 0) {
            ramp_delay = 30;
        }
    }

    if (ramp_delay) {
        // first use old gains
        SampleUtil::copy4WithGain(pInOut,
                                  pInOut, old_dry,
                                  m_pLowBuf, old_low,
                                  m_pBandBuf, old_mid,
                                  m_pHighBuf, old_high,
                                  ramp_delay);
        // Now ramp the remaining frames
        SampleUtil::copy4WithRampingGain(&pInOut[ramp_delay],
                                         &pInOut[ramp_delay], old_dry, fDry,
                                         &m_pLowBuf[ramp_delay], old_low, fLow,
                                         &m_pBandBuf[ramp_delay], old_mid, fMid,
                                         &m_pHighBuf[ramp_delay], old_high, fHigh,
                                         iBufferSize - ramp_delay);
    } else if (fLow != old_low ||
            fMid != old_mid ||
            fHigh != old_high ||
            fDry != old_dry) {
        SampleUtil::copy4WithRampingGain(pInOut,
                                         pInOut, old_dry, fDry,
                                         m_pLowBuf, old_low, fLow,
                                         m_pBandBuf, old_mid, fMid,
                                         m_pHighBuf, old_high, fHigh,
                                         iBufferSize);
    } else {
        SampleUtil::copy4WithGain(pInOut,
                                  pInOut, fDry,
                                  m_pLowBuf, fLow,
                                  m_pBandBuf, fMid,
                                  m_pHighBuf, fHigh,
                                  iBufferSize);
    }

    old_low = fLow;
    old_mid = fMid;
    old_high = fHigh;
    old_dry = fDry;
}
