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

class ControlEngine;
class EngineBuffer;
class EngineVolume;
class EngineChannel;
class EngineClipping;
class EngineFlanger;
class EngineVUmeter;

/**
  *@author 
  */

class EngineMaster : public EngineObject
{
public:
    EngineMaster(DlgMaster *master_dlg, DlgCrossfader *crossfader_dlg,
                 EngineBuffer *buffer1, EngineBuffer *buffer2,
                 EngineChannel *, EngineChannel *, EngineFlanger *, const char *group);
    ~EngineMaster();
    void notify(double) {};
    CSAMPLE *process(const CSAMPLE *, const int);
private:
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineVolume *volume, *head_volume; 
    EngineClipping *clipping, *head_clipping;
    EngineFlanger *flanger;
    EngineVUmeter *vumeter;

    ControlEngine *crossfader, *head_mix, *pfl1, *pfl2, *flanger1, *flanger2;
    CSAMPLE *out, *tmp, *tmp2, *tmp3, *tmp4;
    bool master1, master2;
};

#endif
