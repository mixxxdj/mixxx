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

EngineMaster::EngineMaster(DlgMaster *master_dlg, DlgCrossfader *crossfader_dlg,
                           EngineBuffer *_buffer1, EngineBuffer *_buffer2,
                           EngineChannel *_channel1, EngineChannel *_channel2,
                           const char *group)
{
    buffer1 = _buffer1;
    buffer2 = _buffer2;
    channel1 = _channel1;
    channel2 = _channel2;

    // Crossfader
    ConfigKey k(group, "crossfader");
    crossfader = new ControlPotmeter(&k,-1.,1.);
    connect(crossfader_dlg->SliderCrossfader, SIGNAL(valueChanged(int)), crossfader, SLOT(slotSetPosition(int)));
    connect(crossfader, SIGNAL(updateGUI(int)), crossfader_dlg->SliderCrossfader, SLOT(setValue(int)));

    // Master volume
    volume = new EngineVolume(group);
    connect(master_dlg->KnobVolume, SIGNAL(valueChanged(int)), volume->potmeter, SLOT(slotSetPosition(int)));
    connect(volume->potmeter, SIGNAL(updateGUI(int)), master_dlg->KnobVolume, SLOT(setValue(int)));

    // Clipping:
    clipping = new EngineClipping(master_dlg->BulbClipping);

    out = new CSAMPLE[MAX_BUFFER_LEN];
    out2 = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineMaster::~EngineMaster()
{ 
    delete crossfader;
    delete volume;
    delete clipping;
    delete out;
    delete out2;
}

CSAMPLE *EngineMaster::process(const CSAMPLE *, const int buffer_size) {
    CSAMPLE *sampLeft=0, *sampRight=0;

    if (left)
    {
        CSAMPLE *temp_1 = buffer1->process(0, buffer_size);
	sampLeft = channel1->process(temp_1,buffer_size);
    } 

    if (right)
    {
        CSAMPLE *temp_2 = buffer2->process(0, buffer_size);
        sampRight = channel2->process(temp_2,buffer_size);
    }

    // Crossfader:
    FLOAT_TYPE cf_val = crossfader->getValue();
    FLOAT_TYPE c1_gain, c2_gain;
    c2_gain = 0.5*(cf_val+1.);
    c1_gain = 0.5*(-cf_val+1.);
    //qDebug("c1_gain: %f, c2_gain: %f",c1_gain,c2_gain);

    for (int i=0; i<buffer_size; i++)
    {
        out[i] = 0;
        if (left)
            out[i] += sampLeft[i]*c1_gain;
        if (right)
            out[i] += sampRight[i]*c2_gain;
    }

    // Master volume:
    out2 = volume->process(out, buffer_size);

    // Clipping
    out = clipping->process(out2, buffer_size);

    return out;
}

void EngineMaster::slotChannelLeft(bool toggle)
{
    left = toggle;
}

void EngineMaster::slotChannelRight(bool toggle)
{
    right = toggle;
}

