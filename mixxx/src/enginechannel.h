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

class EnginePregain;
class EngineBuffer;
class EngineFilterBlock;
class EngineClipping;
class EngineVolume;
class EngineFlanger;
class EngineVuMeter;
class EngineTemporal;
class ControlPushButton;

/**
  *@author 
  */

class EngineChannel : public EngineObject  
{
public:
    EngineChannel(const char *group);
    ~EngineChannel();
    void notify(double) {};
    ControlPushButton *getPFL();
    void setVisual(EngineBuffer *pEngineBuffer);
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

private:
    EngineTemporal *m_pEngineTemporal;
    EngineVolume *m_pEngineTemporalVolume;

    EnginePregain *pregain;
    EngineFilterBlock *filter;
    EngineClipping *clipping;
    EngineVolume *volume;
    EngineVuMeter *vumeter;
    ControlPushButton *pfl;
};

#endif
