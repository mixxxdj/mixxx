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

#include "engineflanger.h"
// #include "qradiobutton.h"
#include "wknob.h"
#include "wpushbutton.h"
// #include <qpushbutton.h>
#include "dlgflanger.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "controlengine.h"


/*----------------------------------------------------------------
  A flanger effect.
  The flanger is controlled by the following variables:
    average_delay_length - The average length of the delay, which is modulated by the LFO.
    LFOperiod - the period of LFO given in samples.
    LFOamplitude - the amplitude of the modulation of the delay length.
    depth - the depth of the flanger, controlled by a ControlPotmeter.
  ----------------------------------------------------------------*/
EngineFlanger::EngineFlanger(DlgFlanger *dlg_flanger, const char *group)
{
    // Init. buffers:
    process_buffer = new CSAMPLE[MAX_BUFFER_LEN];
    delay_buffer = new CSAMPLE[max_delay+1];

    // Init. potmeters
    ControlPotmeter *p = new ControlPotmeter(ConfigKey(group, "lfo_depth"), 0., 1.);
    p->setWidget(dlg_flanger->DialDepth);
    potmeterDepth = new ControlEngine(p);
    
    p = new ControlPotmeter(ConfigKey(group, "lfo_delay"), 50., 1000.);
    p->setWidget(dlg_flanger->DialDelay);
    potmeterDelay = new ControlEngine(p);

    p = new ControlPotmeter(ConfigKey(group, "lfo_period"), 5000., 80000.);
    p->setWidget(dlg_flanger->DialPeriod);
    potmeterLFOperiod = new ControlEngine(p);

    // Init. channel selects:
    ControlPushButton *p2 =  new ControlPushButton( ConfigKey(group, "FlangerA"), dlg_flanger->BulbChannelA);
    p2->setWidget(dlg_flanger->PushButtonChA);
    pushbuttonChannelA = new ControlEngine(p2);
    
    p2 =  new ControlPushButton( ConfigKey(group, "FlangerB"), dlg_flanger->BulbChannelB);
    p2->setWidget(dlg_flanger->PushButtonChB);
    pushbuttonChannelB = new ControlEngine(p2);

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
    delete pushbuttonChannelA;
    delete pushbuttonChannelB;
    delete [] process_buffer;
    delete [] delay_buffer;
}

CSAMPLE *EngineFlanger::process(const CSAMPLE *source, const int buffer_size)
{
    CSAMPLE delayed_sample,prev,next;
    FLOAT_TYPE frac;

    for (int i=0; i<buffer_size; i++) 
    {
        // put sample into delay buffer:
        delay_buffer[delay_pos] = source[i];
        delay_pos++;
        if (delay_pos >= max_delay)
            delay_pos=0;

        // Update the LFO to find the current delay:
        time++;
        if (time==potmeterLFOperiod->get()) time=0;
        delay = average_delay_length + LFOamplitude*sin( two_pi*((FLOAT_TYPE) time)/((FLOAT_TYPE) potmeterLFOperiod->get()) );

        // Make a linear interpolation to find the delayed sample:
        prev = delay_buffer[(delay_pos-(int)delay+max_delay-1) % max_delay];
        next = delay_buffer[(delay_pos-(int)delay+max_delay) % max_delay];
        frac = delay - floor(delay);
        delayed_sample = prev + frac*(next-prev);

        // Take the sample from the delay buffer and mix it with the source buffer:
        process_buffer[i] = source[i] + potmeterDepth->get()*delayed_sample;
    }
    return process_buffer;
}

