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
class EngineFilterBlock;
class EngineClipping;
class EngineVolume;
class EngineFlanger;
class EngineVuMeter;
class EngineRealSearch;
class EngineBuffer;
class ControlPushButton;

/**
  *@author 
  */

class EngineChannel : public EngineObject  
{
public:
    EngineChannel(const char *group, EngineBuffer *pBuffer);
    ~EngineChannel();
    void notify(double) {};
    ControlPushButton *getPFL();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

private:
    EnginePregain *pregain;
    EngineFilterBlock *filter;
    EngineClipping *clipping;
    EngineVolume *volume;
    EngineVuMeter *vumeter;
    EngineRealSearch *m_pRealSearch;
    ControlPushButton *pfl;
};

#endif
