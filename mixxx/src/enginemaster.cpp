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
#include "configobject.h"
#include <wslider.h>
#include <qcheckbox.h>

EngineMaster::EngineMaster(DlgMaster *master_dlg, DlgCrossfader *crossfader_dlg,
                           DlgChannel *channel1_dlg, DlgChannel *channel2_dlg,
                           EngineBuffer *_buffer1, EngineBuffer *_buffer2,
                           EngineChannel *_channel1, EngineChannel *_channel2,
			   EngineFlanger *_flanger,
                           const char *group)
{
    buffer1 = _buffer1;
    buffer2 = _buffer2;
    channel1 = _channel1;
    channel2 = _channel2;
    flanger = _flanger;

    // Crossfader
    crossfader = new ControlPotmeter(ConfigKey(group, "crossfader"),-1.,1.);
    connect(crossfader_dlg->SliderCrossfader, SIGNAL(valueChanged(int)), crossfader, SLOT(slotSetPosition(int)));
    connect(crossfader, SIGNAL(updateGUI(int)), crossfader_dlg->SliderCrossfader, SLOT(setValue(int)));

    // Master volume
    volume = new EngineVolume(ConfigKey(group,"volume"));
    connect(master_dlg->KnobVolume, SIGNAL(valueChanged(int)), volume->potmeter, SLOT(slotSetPosition(int)));
    connect(volume->potmeter, SIGNAL(updateGUI(int)), master_dlg->KnobVolume, SLOT(setValue(int)));

    // Clipping
    clipping = new EngineClipping(master_dlg->BulbClipping);
    
    // Headphone volume
    head_volume = new EngineVolume(ConfigKey(group, "headvolume"));
    connect(master_dlg->KnobHeadVol, SIGNAL(valueChanged(int)), head_volume->potmeter, SLOT(slotSetPosition(int)));
    connect(head_volume->potmeter, SIGNAL(updateGUI(int)), master_dlg->KnobHeadVol, SLOT(setValue(int)));
   
    // Headphone mix (left/right)
    head_mix = new ControlPotmeter(ConfigKey(group, "head_mix"),-1.,1.);
    connect(master_dlg->KnobHeadLR, SIGNAL(valueChanged(int)), head_mix, SLOT(slotSetPosition(int)));
    connect(head_mix, SIGNAL(updateGUI(int)), master_dlg->KnobHeadLR, SLOT(setValue(int)));

    // Headphone Clipping
    head_clipping = new EngineClipping(0);

    // Channel 1 and 2 head
    connect(channel1_dlg->CheckBoxHead, SIGNAL(stateChanged(int)), this, SLOT(slotChannelHead1(int)));
    connect(channel2_dlg->CheckBoxHead, SIGNAL(stateChanged(int)), this, SLOT(slotChannelHead2(int)));
    head1=false;
    head2=false;

    out = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineMaster::~EngineMaster()
{ 
    delete crossfader;
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
	if (flanger->channel_A) 
            sampMaster1 = flanger->process(temp_2, buffer_size);
	else
	    sampMaster1 = temp_2;
    } 

    if (master2)
    {
        CSAMPLE *temp_1 = buffer2->process(0, buffer_size);
        CSAMPLE *temp_2 = channel2->process(temp_1,buffer_size);
	if (flanger->channel_B) 
            sampMaster2 = flanger->process(temp_2, buffer_size);
	else
	    sampMaster2 = temp_2;
    }

    //
    // Output channel:
    //
    
    // Crossfader
    FLOAT_TYPE cf_val = crossfader->getValue();
    FLOAT_TYPE c1_gain, c2_gain;
    c2_gain = 0.5*(cf_val+1.);
    c1_gain = 0.5*(-cf_val+1.);

    if (master1 && !head1 && master2 && !head2)
        for (int i=0; i<buffer_size; i++)
            out[i] = sampMaster1[i]*c1_gain + sampMaster2[i]*c2_gain;
    else if (master1 && !head1)
        for (int i=0; i<buffer_size; i++)
            out[i] = sampMaster1[i]*c1_gain;
    else if (master2 && !head2)
        for (int i=0; i<buffer_size; i++)
            out[i] = sampMaster2[i]*c2_gain;
    else
        for (int i=0; i<buffer_size; i++)
            out[i] = 0.;

    // Master volume
    tmp = volume->process(out, buffer_size);

    // Clipping
    tmp2 = clipping->process(tmp, buffer_size);

    //
    // Headphone channel:
    //
    if (CH_HEAD>0)
    {
        // Head phone left/right mix
        cf_val = head_mix->getValue();
        c1_gain = 0.5*(-cf_val+1.);
        c2_gain = 0.5*(cf_val+1.);

        if (master1 && head1 && master2 && head2)
            for (int i=0; i<buffer_size; i++)
                out[i] = tmp2[i]*c1_gain + sampMaster1[i]*c2_gain + sampMaster2[i]*c2_gain;
        else if (master1 && head1)
            for (int i=0; i<buffer_size; i++)
                out[i] += tmp2[i]*c1_gain + sampMaster1[i]*c2_gain;
        else if (master2 && head2)
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
    {
        int j=0;
        for (int i=0; i<buffer_size; i+=2)
        {
            // Interleave the output and the headphone channels
            out[j  ] = tmp2[i  ];
            out[j+1] = tmp2[i+1];
            out[j+2] = tmp3[i  ];
            out[j+3] = tmp3[i+1];
            j+=4;
        }
    }   
    return out;
}

void EngineMaster::slotChannelMaster1(bool toggle)
{
    master1 = toggle;
}

void EngineMaster::slotChannelMaster2(bool toggle)
{
    master2 = toggle;
}

void EngineMaster::slotChannelHead1(int toggle)
{
    if (toggle == 2)
        head1 = true;
    else
        head1 = false;
}

void EngineMaster::slotChannelHead2(int toggle)
{
    if (toggle == 2)
        head2 = true;
    else
        head2 = false;
}

