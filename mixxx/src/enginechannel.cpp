/***************************************************************************
                          enginechannel.cpp  -  description
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

#include "enginechannel.h"
#include "qslider.h"
#include "controllogpotmeter.h"

EngineChannel::EngineChannel(DlgChannel *dlg, MidiObject *midi, int midiPregain, int midiLow,
                             int midiBand, int midiHigh, int midiVolume)
{
  // Pregain:
  pregain = new EnginePregain(midiPregain, midi);
  connect(dlg->DialGain, SIGNAL(valueChanged(int)), pregain->pregainpot, SLOT(slotSetPosition(int)));
  connect(pregain->pregainpot, SIGNAL(recievedMidi(int)), dlg->DialGain, SLOT(setValue(int)));
  connect(pregain->pregainpot, SIGNAL(valueChanged(FLOAT_TYPE)), pregain, SLOT(slotUpdate(FLOAT_TYPE)));

  // Filters:
  filter = new EngineFilterBlock(dlg->DialFilterLow, midiLow,
                                 dlg->DialFilterMiddle, midiBand,
                                 dlg->DialFilterHigh , midiHigh, midi);

  // Clipping:
  clipping = new EngineClipping(dlg->BulbClipping);

  // Volume control:
  volume = new EnginePregain(midiVolume, midi);
  connect(dlg->SliderVolume, SIGNAL(valueChanged(int)), volume->pregainpot, SLOT(slotSetPosition(int)));
  connect(volume->pregainpot, SIGNAL(recievedMidi(int)), dlg->SliderVolume, SLOT(setValue(int)));
  connect(volume->pregainpot, SIGNAL(valueChanged(FLOAT_TYPE)), volume, SLOT(slotUpdate(FLOAT_TYPE)));
}

EngineChannel::~EngineChannel(){
  delete pregain;
  delete filter;
  delete clipping;
  delete volume;
}

CSAMPLE *EngineChannel::process(const CSAMPLE* source, const int buffer_size) {
  CSAMPLE *temp  = pregain->process(source, buffer_size);
  CSAMPLE *temp2 = filter->process(temp, buffer_size);
  temp = clipping->process(temp2, buffer_size);
  temp2 = volume->process(temp, buffer_size);

  return temp2;
}
