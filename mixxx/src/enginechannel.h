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

class ControlEngine;
class EnginePregain;
class EngineFilterBlock;
class EngineClipping;
class EngineVolume;
class EngineFlanger;
class EngineVuMeter;

/**
  *@author 
  */

class EngineChannel : public EngineObject  {
public:
    EngineChannel(const char *group);
    ~EngineChannel();
    void notify(double) {};
    ControlEngine *getPFL();
    CSAMPLE *process(const CSAMPLE *, const int);

private:
    EnginePregain *pregain;
    EngineFilterBlock *filter;
    EngineClipping *clipping;
    EngineVolume *volume;
    EngineVuMeter *vumeter;
    ControlEngine *pfl;
};

#endif
