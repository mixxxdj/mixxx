/***************************************************************************
                          engineflanger.cpp  -  description
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

#include <QtDebug>

#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "engine/engineflanger.h"
#include "mathstuff.h"
#include "sampleutil.h"

class EngineFlangerControls {
  public:
    static QSharedPointer<EngineFlangerControls> instance() {
        if (!m_pInstance) {
            QSharedPointer<EngineFlangerControls> ptr(new EngineFlangerControls());
            m_pInstance = ptr;
            return ptr;
        }
        return m_pInstance;
    }

    ~EngineFlangerControls() {
        qDebug() << "~EngineFlangerControls";
        delete m_pPotmeterDepth;
        delete m_pPotmeterDelay;
        delete m_pPotmeterLFOperiod;
    }

    ControlObject* m_pPotmeterDepth;
    ControlObject* m_pPotmeterDelay;
    ControlObject* m_pPotmeterLFOperiod;

  private:
    EngineFlangerControls() {
        m_pPotmeterDepth = new ControlPotmeter(
                ConfigKey("[Flanger]", "lfoDepth"), 0., 1.);
        m_pPotmeterDelay = new ControlPotmeter(
                ConfigKey("[Flanger]", "lfoDelay"), 50., 10000.);
        m_pPotmeterLFOperiod = new ControlPotmeter(
                ConfigKey("[Flanger]", "lfoPeriod"), 50000., 2000000.);
    }
    static QWeakPointer<EngineFlangerControls> m_pInstance;
};

// static
QWeakPointer<EngineFlangerControls> EngineFlangerControls::m_pInstance;

/*----------------------------------------------------------------
   A flanger effect.
   The flanger is controlled by the following variables:
    average_delay_length - The average length of the delay, which is modulated by the LFO.
    LFOperiod - the period of LFO given in samples.
    LFOamplitude - the amplitude of the modulation of the delay length.
    depth - the depth of the flanger, controlled by a ControlPotmeter.
   ----------------------------------------------------------------*/
EngineFlanger::EngineFlanger(const char* group) {
    // Init. buffers:
    m_pDelay_buffer = SampleUtil::alloc(max_delay + 1);
    SampleUtil::clear(m_pDelay_buffer, max_delay+1);

    // Init. potmeters

    // rryan 6/2010 This is gross. The flanger was originally written as this
    // hack that hard-coded the two channels, and while pulling it apart, I have
    // to keep these global [Flanger]-group controls, except there is one
    // EngineFlanger per deck
    m_pControls = EngineFlangerControls::instance();

    // Create an enable key on a per-deck basis.
    m_pFlangerEnable = new ControlPushButton(ConfigKey(group, "flanger"));
    m_pFlangerEnable->setButtonMode(ControlPushButton::TOGGLE);

    // Fixed values of controls:
    m_LFOamplitude = 240;
    m_average_delay_length = 250;

    // Set initial values for vars
    m_delay_pos = 0;
    m_time = 0;
}

EngineFlanger::~EngineFlanger() {
    // Don't delete the controls anymore since we don't know if we created them.
    // delete m_pPotmeterDepth;
    // delete m_pPotmeterDelay;
    // delete m_pPotmeterLFOperiod;

    delete m_pFlangerEnable;

    SampleUtil::free(m_pDelay_buffer);
}

void EngineFlanger::process(const CSAMPLE* pIn,
                            CSAMPLE* pOutput, const int iBufferSize) {
    CSAMPLE delayed_sample,prev,next;
    FLOAT_TYPE frac;

    if (m_pFlangerEnable->get() == 0.0) {
        // SampleUtil handles shortcuts when aliased, and gains of 1.0, etc.
        return SampleUtil::copyWithGain(pOutput, pIn, 1.0f, iBufferSize);
    }

    for (int i=0; i<iBufferSize; ++i) {
        // put sample into delay buffer:
        m_pDelay_buffer[m_delay_pos] = pIn[i];
        m_delay_pos++;
        if (m_delay_pos >= max_delay) {
            m_delay_pos = 0;
        }

        // Update the LFO to find the current delay:
        m_time++;
        if (m_time == m_pControls->m_pPotmeterLFOperiod->get()) {
            m_time = 0;
        }
        FLOAT_TYPE delay = m_average_delay_length + m_LFOamplitude *
                sin(two_pi * ((FLOAT_TYPE)m_time) /
                        ((FLOAT_TYPE)m_pControls->m_pPotmeterLFOperiod->get()));

        // Make a linear interpolation to find the delayed sample:
        prev = m_pDelay_buffer[(m_delay_pos-(int)delay + max_delay - 1) % max_delay];
        next = m_pDelay_buffer[(m_delay_pos-(int)delay + max_delay) % max_delay];
        frac = delay - floor(delay);
        delayed_sample = prev + frac * (next - prev);

        // Take the sample from the delay buffer and mix it with the source buffer:
        pOutput[i] = pIn[i] + m_pControls->m_pPotmeterDepth->get() * delayed_sample;
    }
}
