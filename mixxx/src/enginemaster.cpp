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

EngineMaster::EngineMaster(DlgMaster *master, EngineBuffer *_buffer1, EngineBuffer *_buffer2,
                           EngineChannel *_channel1, EngineChannel *_channel2,
                           int midiCrossfader, int midiVolume, MidiObject *midi){
    buffer1 = _buffer1;
    buffer2 = _buffer2;
    channel1 = _channel1;
    channel2 = _channel2;

    crossfader = new ControlPotmeter("crossfader", midiCrossfader, midi, -1, 1);
    connect(master->SliderCrossfader, SIGNAL(valueChanged(int)), crossfader, SLOT(slotSetPosition(int)));
    connect(crossfader, SIGNAL(recievedMidi(int)), master->SliderCrossfader, SLOT(setValue(int)));

    volume = new EnginePregain(midiVolume, midi);
    connect(master->KnobVolume, SIGNAL(valueChanged(int)), volume->pregainpot, SLOT(slotSetPosition(int)));
    connect(volume->pregainpot, SIGNAL(recievedMidi(int)), master->KnobVolume, SLOT(setValue(int)));
    connect(volume->pregainpot, SIGNAL(valueChanged(FLOAT_TYPE)), volume, SLOT(slotUpdate(FLOAT_TYPE)));

    // Clipping:
    clipping = new EngineClipping(master->BulbClipping);

}

EngineMaster::~EngineMaster(){
    delete crossfader;
    delete volume;
}

CSAMPLE *EngineMaster::process(const CSAMPLE *, const int buffer_size) {
    CSAMPLE *temp_1 = buffer1->process(0, buffer_size);
    CSAMPLE *temp_2 = buffer2->process(0, buffer_size);
    CSAMPLE *temp_3 = channel1->process(temp_1,buffer_size);
    CSAMPLE *temp_4 = channel2->process(temp_2,buffer_size);

    // Crossfader:
    FLOAT_TYPE cf_val = crossfader->getValue();
    FLOAT_TYPE c1_gain, c2_gain;
    c2_gain = 0.5*(cf_val+1.);
    c1_gain = 0.5*(-cf_val+1.);
    //qDebug("c1_gain: %f, c2_gain: %f",c1_gain,c2_gain);

    for (int i=0; i<buffer_size; i++)
        temp_1[i] = temp_3[i]*c1_gain + temp_4[i]*c2_gain;

    // Master volume:
    temp_3 = volume->process(temp_1, buffer_size);

    // Clipping
    temp_1 = clipping->process(temp_3, buffer_size);

    return temp_1;
}





