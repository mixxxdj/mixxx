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

#include "engine/enginevumeter.h"
#include "controlpotmeter.h"
#include "controlobjectslave.h"
#include "sampleutil.h"
#include "util/math.h"

EngineVuMeter::EngineVuMeter(QString group) {
    // The VUmeter widget is controlled via a controlpotmeter, which means
    // that it should react on the setValue(int) signal.
    m_ctrlVuMeter = new ControlPotmeter(ConfigKey(group, "VuMeter"), 0., 1.);
    // left channel VU meter
    m_ctrlVuMeterL = new ControlPotmeter(ConfigKey(group, "VuMeterR"), 0., 1.);
    // right channel VU meter
    m_ctrlVuMeterR = new ControlPotmeter(ConfigKey(group, "VuMeterL"), 0., 1.);

    // Used controlpotmeter as the example used it :/ perhaps someone with more
    // knowledge could use something more suitable...
    m_ctrlPeakIndicator = new ControlPotmeter(ConfigKey(group, "PeakIndicator"),
                                              0., 1.);

    m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate", this);

    // Initialize the calculation:
    reset();
}

EngineVuMeter::~EngineVuMeter()
{
    delete m_ctrlVuMeter;
    delete m_ctrlVuMeterL;
    delete m_ctrlVuMeterR;
    delete m_ctrlPeakIndicator;
}

void EngineVuMeter::process(CSAMPLE* pIn, const int iBufferSize) {
    CSAMPLE fVolSumL, fVolSumR;

    int sampleRate = (int)m_pSampleRate->get();

    bool clipped = SampleUtil::sumAbsPerChannel(&fVolSumL, &fVolSumR, pIn, iBufferSize);
    m_fRMSvolumeSumL += fVolSumL;
    m_fRMSvolumeSumR += fVolSumR;

    m_iSamplesCalculated += iBufferSize/2;

    // Are we ready to update the VU meter?:
    if (m_iSamplesCalculated > (sampleRate/VU_UPDATE_RATE)) {
        doSmooth(m_fRMSvolumeL, log10(SHRT_MAX * m_fRMSvolumeSumL/(m_iSamplesCalculated*1000)+1));
        doSmooth(m_fRMSvolumeR, log10(SHRT_MAX * m_fRMSvolumeSumR/(m_iSamplesCalculated*1000)+1));

        const double epsilon = .0001;

        // Since VU meters are a rolling sum of audio, the no-op checks in
        // ControlObject will not prevent us from causing tons of extra
        // work. Because of this, we use an epsilon here to be gentle on the GUI
        // and MIDI controllers.
        if (fabs(m_fRMSvolumeL - m_ctrlVuMeterL->get()) > epsilon)
            m_ctrlVuMeterL->set(m_fRMSvolumeL);
        if (fabs(m_fRMSvolumeR - m_ctrlVuMeterR->get()) > epsilon)
            m_ctrlVuMeterR->set(m_fRMSvolumeR);

        double fRMSvolume = (m_fRMSvolumeL + m_fRMSvolumeR) / 2.0;
        if (fabs(fRMSvolume - m_ctrlVuMeter->get()) > epsilon)
            m_ctrlVuMeter->set(fRMSvolume);

        // Reset calculation:
        m_iSamplesCalculated = 0;
        m_fRMSvolumeSumL = 0;
        m_fRMSvolumeSumR = 0;
    }

    if (clipped) {
        m_ctrlPeakIndicator->set(1.);
        m_peakDuration = PEAK_DURATION * sampleRate / iBufferSize / 2000;
    } else if (m_peakDuration <= 0) {
        m_ctrlPeakIndicator->set(0.);
    } else {
        --m_peakDuration;
    }
}

void EngineVuMeter::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    pGroupFeatures->rms_volume_sum = (m_fRMSvolumeL + m_fRMSvolumeR) / 2.0;
    pGroupFeatures->has_rms_volume_sum = true;
}

void EngineVuMeter::doSmooth(CSAMPLE &currentVolume, CSAMPLE newVolume)
{
    if (currentVolume > newVolume)
        currentVolume -= DECAY_SMOOTHING * (currentVolume - newVolume);
    else
        currentVolume += ATTACK_SMOOTHING * (newVolume - currentVolume);
    if (currentVolume < 0)
        currentVolume=0;
    if (currentVolume > 1.0)
        currentVolume=1.0;
}

void EngineVuMeter::reset() {
    m_ctrlVuMeter->set(0);
    m_ctrlVuMeterL->set(0);
    m_ctrlVuMeterR->set(0);
    m_ctrlPeakIndicator->set(0);

    m_iSamplesCalculated = 0;
    m_fRMSvolumeL = 0;
    m_fRMSvolumeSumL = 0;
    m_fRMSvolumeR = 0;
    m_fRMSvolumeSumR = 0;
    m_peakDuration = 0;
}
