/***************************************************************************
                          enginemaster.h  -  description
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

#ifndef ENGINEMASTER_H
#define ENGINEMASTER_H

#include "engineobject.h"
#include "enginebuffer.h"
#include "dlgmaster.h"
#include "enginepregain.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "controlpotmeter.h"

/**
  *@author 
  */

class EngineMaster : public EngineObject  {
private:
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EnginePregain *volume;
    ControlPotmeter *crossfader;
    EngineClipping *clipping;
public: 
	EngineMaster(DlgMaster *master, EngineBuffer *buffer1, EngineBuffer *buffer2,
                 EngineChannel *, EngineChannel *,
                 int midiCrossfader, int midiVolume, MidiObject *midi);
	~EngineMaster();
    CSAMPLE *process(const CSAMPLE *, const int);
};

#endif
