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
#include "engineflanger.h"
#include "mathstuff.h"
#include "sampleutil.h"


/*----------------------------------------------------------------
   A flanger effect.
   The flanger is controlled by the following variables:
    average_delay_length - The average length of the delay, which is modulated by the LFO.
    LFOperiod - the period of LFO given in samples.
    LFOamplitude - the amplitude of the modulation of the delay length.
    depth - the depth of the flanger, controlled by a ControlPotmeter.
   ----------------------------------------------------------------*/
EngineFlanger::EngineFlanger(const char * group)
{
    // Init. buffers:
    delay_buffer = SampleUtil::alloc(max_delay + 1);
    SampleUtil::applyGain(delay_buffer, 0.0f, max_delay+1);

    // Init. potmeters

    // rryan 6/2010 This is gross. The flanger was originally written as this
    // hack that hard-coded the two channels, and while pulling it apart, I have
    // to keep these global [Flanger]-group controls, except there is one
    // EngineFlanger per deck, so create these controls if they don't exist,
    // otherwise look them up.

    potmeterDepth = ControlObject::getControl(ConfigKey("[Flanger]", "lfoDepth"));
    potmeterDelay = ControlObject::getControl(ConfigKey("[Flanger]", "lfoDelay"));
    potmeterLFOperiod = ControlObject::getControl(ConfigKey("[Flanger]", "lfoPeriod"));

    if (potmeterDepth == NULL)
        potmeterDepth = new ControlPotmeter(ConfigKey("[Flanger]", "lfoDepth"), 0., 1.);
    if (potmeterDelay == NULL)
        potmeterDelay = new ControlPotmeter(ConfigKey("[Flanger]", "lfoDelay"), 50., 10000.);
    if (potmeterLFOperiod == NULL)
        potmeterLFOperiod = new ControlPotmeter(ConfigKey("[Flanger]", "lfoPeriod"), 50000., 2000000.);

    // Create an enable key on a per-deck basis.
    flangerEnable = new ControlPushButton(ConfigKey(group, "flanger"));
    flangerEnable->setButtonMode(ControlPushButton::TOGGLE);

    // Fixed values of controls:
    LFOamplitude = 240;
    average_delay_length = 250;

    // Set initial values for vars
    delay_pos=0;
    time = 0;
}

EngineFlanger::~EngineFlanger()
{
    // Don't delete the controls anymore since we don't know if we created them.
    // delete potmeterDepth;
    // delete potmeterDelay;
    // delete potmeterLFOperiod;

    delete flangerEnable;

    SampleUtil::free(delay_buffer);
}

void EngineFlanger::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    CSAMPLE delayed_sample,prev,next;
    FLOAT_TYPE frac;

    if (flangerEnable->get() == 0.0f) {
        // SampleUtil handles shortcuts when aliased, and gains of 1.0, etc.
        return SampleUtil::copyWithGain(pOutput, pIn, 1.0f, iBufferSize);
    }

    for (int i=0; i<iBufferSize; ++i)
    {
        // put sample into delay buffer:
        delay_buffer[delay_pos] = pIn[i];
        delay_pos++;
        if (delay_pos >= max_delay)
            delay_pos=0;

        // Update the LFO to find the current delay:
        time++;
        if (time==potmeterLFOperiod->get()) time=0;
        delay = average_delay_length + LFOamplitude *sin( two_pi * ((FLOAT_TYPE) time)/((FLOAT_TYPE) potmeterLFOperiod->get()) );

        // Make a linear interpolation to find the delayed sample:
        prev = delay_buffer[(delay_pos-(int)delay+max_delay-1) % max_delay];
        next = delay_buffer[(delay_pos-(int)delay+max_delay) % max_delay];
        frac = delay - floor(delay);
        delayed_sample = prev + frac*(next-prev);

        // Take the sample from the delay buffer and mix it with the source buffer:
        pOutput[i] = pIn[i] + potmeterDepth->get()*delayed_sample;
    }
}
