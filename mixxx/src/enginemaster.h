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
#include "dlgcrossfader.h"
#include "enginepregain.h"
#include "enginechannel.h"
#include "engineclipping.h"
#include "controlpotmeter.h"

/**
  *@author 
  */

class EngineMaster : public EngineObject  {
  Q_OBJECT
private:
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EnginePregain *volume;
    ControlPotmeter *crossfader;
    EngineClipping *clipping;
    CSAMPLE *out, *out2;
    bool left, right;
public:
    EngineMaster(DlgMaster *master_dlg, DlgCrossfader *crossfader_dlg,
                 EngineBuffer *buffer1, EngineBuffer *buffer2,
                 EngineChannel *, EngineChannel *,
                 int midiCrossfader, int midiVolume, MidiObject *midi);
    ~EngineMaster();
    CSAMPLE *process(const CSAMPLE *, const int);
public slots:
    void slotChannelLeft(bool toggle);
    void slotChannelRight(bool toggle);
};

#endif
