/***************************************************************************
                          enginemaster.cpp  -  description
                             -------------------
    begin                : Sun Apr 28 2002
    copyright            : (C) 2002 by 
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

#include "enginemaster.h"
#include "enginebuffer.h"
#include "enginevolume.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "engineflanger.h"
#include "enginevumeter.h"
#include "configobject.h"
#include "controlpotmeter.h"
#include "controlengine.h"

EngineMaster::EngineMaster(EngineBuffer *_buffer1, EngineBuffer *_buffer2,
                           EngineChannel *_channel1, EngineChannel *_channel2,
                           EngineFlanger *_flanger,
                           const char *group)
{
    buffer1 = _buffer1;
    buffer2 = _buffer2;
    channel1 = _channel1;
    channel2 = _channel2;
    flanger = _flanger;

    // Defaults
    master1 = true;
    master2 = true;

    // Crossfader
    ControlPotmeter *p = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);
    crossfader = new ControlEngine(p);

    // Balance
    p = new ControlPotmeter(ConfigKey(group, "balance"), -1., 1.);
    m_pBalance = new ControlEngine(p);
            
    // Master volume
    volume = new EngineVolume(ConfigKey(group,"volume"));
    
    // Clipping
    clipping = new EngineClipping(group);

    // VU meter:
    vumeter = new EngineVuMeter(group);

    // Headphone volume
    head_volume = new EngineVolume(ConfigKey(group, "headVolume"));
   
    // Headphone mix (left/right)
    p = new ControlPotmeter(ConfigKey(group, "headMix"),-1.,1.);
    head_mix = new ControlEngine(p);

    // Headphone Clipping
    head_clipping = new EngineClipping("");

    pfl1 = channel1->getPFL();
    pfl2 = channel2->getPFL();

    flanger1 = flanger->getButtonA();
    flanger2 = flanger->getButtonB();
        
    out = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineMaster::~EngineMaster()
{ 
    delete crossfader;
    delete m_pBalance;
    delete head_mix;
    delete volume;
    delete head_volume;
    delete clipping;
    delete head_clipping;
    delete [] out;
}

CSAMPLE *EngineMaster::process(const CSAMPLE *, const int buffer_size)
{
    CSAMPLE *sampMaster1=0, *sampMaster2=0;

    //
    // Process the buffer, the channels and the effects:
    //

    if (master1)
    {
        CSAMPLE *temp_1 = buffer1->process(0, buffer_size);
        CSAMPLE *temp_2 = channel1->process(temp_1,buffer_size);
        if (flanger1->get()==1.)
            sampMaster1 = flanger->process(temp_2, buffer_size);
        else
            sampMaster1 = temp_2;
    } 

    if (master2)
    {
        CSAMPLE *temp_1 = buffer2->process(0, buffer_size);
        CSAMPLE *temp_2 = channel2->process(temp_1,buffer_size);
        if (flanger1->get()==0. && flanger2->get()==1.) 
            sampMaster2 = flanger->process(temp_2, buffer_size);
        else
            sampMaster2 = temp_2;
    }

    //
    // Output channel:
    //
    
    // Crossfader
    FLOAT_TYPE cf_val = crossfader->get();;
    FLOAT_TYPE c1_gain, c2_gain;
    c2_gain = 0.5*(cf_val+1.);
    c1_gain = 0.5*(-cf_val+1.);

    if (master1 && pfl1->get()==0. && master2 && pfl2->get()==0.)
        for (int i=0; i<buffer_size; i++)
            out[i] = sampMaster1[i]*c1_gain + sampMaster2[i]*c2_gain;
    else if (master1 && pfl1->get()==0.)
        for (int i=0; i<buffer_size; i++)
            out[i] = sampMaster1[i]*c1_gain;
    else if (master2 && pfl2->get()==0.)
        for (int i=0; i<buffer_size; i++)
            out[i] = sampMaster2[i]*c2_gain;
    else
        for (int i=0; i<buffer_size; i++)
            out[i] = 0.;

    // Master volume
    tmp = volume->process(out, buffer_size);

    // Clipping
    tmp2 = clipping->process(tmp, buffer_size);

    // Update VU meter (it does not return anything):
    if (vumeter!=0)
        vumeter->process(tmp2, buffer_size);

    //
    // Headphone channel:
    //
//    if (CH_HEAD>0)
    {
        // Head phone left/right mix
        cf_val = head_mix->get();
        c1_gain = 0.5*(-cf_val+1.);
        c2_gain = 0.5*(cf_val+1.);

        if (master1 && pfl1->get()==1. && master2 && pfl2->get()==1.)
            for (int i=0; i<buffer_size; i++)
                out[i] = tmp2[i]*c1_gain + sampMaster1[i]*c2_gain + sampMaster2[i]*c2_gain;
        else if (master1 && pfl1->get()==1.)
            for (int i=0; i<buffer_size; i++)
                out[i] += tmp2[i]*c1_gain + sampMaster1[i]*c2_gain;
        else if (master2 && pfl2->get()==1.)
            for (int i=0; i<buffer_size; i++)
                out[i] += tmp2[i]*c1_gain + sampMaster2[i]*c2_gain;
        else
            for (int i=0; i<buffer_size; i++)
                out[i] += tmp2[i]*c1_gain;

        // Master volume
        tmp = head_volume->process(out, buffer_size);

        // Clipping
        tmp3 = head_clipping->process(tmp, buffer_size);
    }

    // Interleave samples
/*
    if (CH_HEAD==0)
    {
        int j=0;
        for (int i=0; i<buffer_size; i+=2)
        {
            out[j  ] = tmp2[i  ];
            out[j+1] = tmp2[i+1];
            out[j+2] = 0.;
            out[j+3] = 0.;
            j+=4;
        }
    }
    else
*/
    {
        int j=0;

        // Balance values
        float balright = 1.;
        float balleft = 1.;
        float bal = m_pBalance->get();
        if (bal>0.)
            balleft -= bal;
        else if (bal<0.)
            balright += bal;
        
        for (int i=0; i<buffer_size; i+=2)
        {
            // Interleave the output and the headphone channels, and perform balancing on main out
            out[j  ] = tmp2[i  ]*balleft;
            out[j+1] = tmp2[i+1]*balright;
            out[j+2] = tmp3[i  ];
            out[j+3] = tmp3[i+1];
            j+=4;
        }
    }   
    return out;
}
