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

#include "controlpushbutton.h"
#include "controlpotmeter.h"
#include "engineflanger.h"
#include "mathstuff.h"


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
    delay_buffer = new CSAMPLE[max_delay+1];
    for (int i=0; i<max_delay+1; ++i)
        delay_buffer[i] = 0.;

    // Init. potmeters
    potmeterDepth = new ControlPotmeter(ConfigKey(group, "lfoDepth"), 0., 1.);
    potmeterDelay = new ControlPotmeter(ConfigKey(group, "lfoDelay"), 50., 10000.);
    potmeterLFOperiod = new ControlPotmeter(ConfigKey(group, "lfoPeriod"), 50000., 2000000.);

    // Init. channel selects:
    pushbuttonFlangerCh1 =  new ControlPushButton(ConfigKey("[Channel1]", "flanger"));
    pushbuttonFlangerCh1->setToggleButton(true);
    pushbuttonFlangerCh2 =  new ControlPushButton(ConfigKey("[Channel2]", "flanger"));
    pushbuttonFlangerCh2->setToggleButton(true);

    // Fixed values of controls:
    LFOamplitude = 240;
    average_delay_length = 250;

    // Set initial values for vars
    delay_pos=0;
    time = 0;
}

EngineFlanger::~EngineFlanger()
{
    delete potmeterDepth;
    delete potmeterDelay;
    delete potmeterLFOperiod;
    delete pushbuttonFlangerCh1;
    delete pushbuttonFlangerCh2;
    delete [] delay_buffer;
}

ControlPushButton * EngineFlanger::getButtonCh1()
{
    return pushbuttonFlangerCh1;
}

ControlPushButton * EngineFlanger::getButtonCh2()
{
    return pushbuttonFlangerCh2;
}

void EngineFlanger::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    CSAMPLE delayed_sample,prev,next;
    FLOAT_TYPE frac;

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

