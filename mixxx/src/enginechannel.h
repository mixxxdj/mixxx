/***************************************************************************
                          enginechannel.h  -  description
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

#ifndef ENGINECHANNEL_H
#define ENGINECHANNEL_H

#include "engineobject.h"
#include "engineclipping.h"
#include "enginepregain.h"
#include "enginefilterblock.h"
#include "controllogpotmeter.h"
#include "dlgchannel.h"
#include "midiobject.h"

/**
  *@author 
  */

class EngineChannel : public EngineObject  {
public:
	EngineChannel(DlgChannel *dlg, MidiObject *midi, int midiPregain, int midiLow,
                  int midiBand, int midiHigh, int midiVolume);
	~EngineChannel();
    CSAMPLE *process(const CSAMPLE *, const int);
private:
    EnginePregain* pregain;
    EngineFilterBlock* filter;
    EngineClipping* clipping;
    EnginePregain* volume;
};

#endif
