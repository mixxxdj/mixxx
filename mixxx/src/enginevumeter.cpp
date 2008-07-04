/***************************************************************************
                          enginevumeter.cpp  -  description
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

#ifdef __WIN32__
#pragma intrinsic(fabs)
#endif

#include "enginevumeter.h"
#include "controlpotmeter.h"

EngineVuMeter::EngineVuMeter(const char * group)
{
    // The VUmeter widget is controlled via a controlpotmeter, which means
    // that it should react on the setValue(int) signal.
    m_ctrlVuMeter = new ControlPotmeter(ConfigKey(group, "VuMeter"), 0., 1.);
    m_ctrlVuMeter->set(0);
    // left channel VU meter
    m_ctrlVuMeterL = new ControlPotmeter(ConfigKey(group, "VuMeterL"), 0., 1.);
    m_ctrlVuMeterL->set(0);
    // right channel VU meter
    m_ctrlVuMeterR = new ControlPotmeter(ConfigKey(group, "VuMeterR"), 0., 1.);
    m_ctrlVuMeterR->set(0);

    // Initialize the calculation:
    m_iSamplesCalculated = 0;
    m_fRMSvolumeL = 0;
    m_fRMSvolumeR = 0;
}

EngineVuMeter::~EngineVuMeter()
{
    delete m_ctrlVuMeter;
    delete m_ctrlVuMeterL;
    delete m_ctrlVuMeterR;
}

void EngineVuMeter::process(const CSAMPLE * pIn, const CSAMPLE *, const int iBufferSize)
{
    // Calculate the summed absolute volume
    for (int i=0; i<iBufferSize/2; ++i)
    {
        m_fRMSvolumeSumL += fabsf(pIn[2*i]);
        m_fRMSvolumeSumR += fabsf(pIn[2*i+1]);
    }

    m_iSamplesCalculated += iBufferSize/2;

    // Are we ready to update the VU meter?:
    if (m_iSamplesCalculated*2 > (44100/UPDATE_RATE) )
    {
        m_fRMSvolumeL = log10(m_fRMSvolumeSumL/(m_iSamplesCalculated*1000)+1);
        m_fRMSvolumeR = log10(m_fRMSvolumeSumR/(m_iSamplesCalculated*1000)+1);
        m_ctrlVuMeterL->set( math_min(1.0, math_max(0.0, m_fRMSvolumeL)) );
        m_ctrlVuMeterR->set( math_min(1.0, math_max(0.0, m_fRMSvolumeR)) );

        FLOAT_TYPE m_fRMSvolume = (m_fRMSvolumeL + m_fRMSvolumeR) / 2;
        FLOAT_TYPE m_fRMSvolumePrev = m_fRMSvolume;
        FLOAT_TYPE smoothFactor;
        // Smooth the output
        smoothFactor = (m_fRMSvolumePrev > m_fRMSvolume) ? DECAY_SMOOTHING : ATTACK_SMOOTHING;
        m_fRMSvolume = m_fRMSvolumePrev + smoothFactor * (m_fRMSvolume - m_fRMSvolumePrev);
        m_ctrlVuMeter->set( math_min(1.0, math_max(0.0, m_fRMSvolume)) );
        // Reset calculation:
        m_iSamplesCalculated = 0;
        m_fRMSvolumeSumL = 0;
        m_fRMSvolumeSumR = 0;
    }
}
