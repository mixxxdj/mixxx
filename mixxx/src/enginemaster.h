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

class EngineBuffer;
class EngineVolume;
class EngineChannel;
class EngineClipping;
class EngineFlanger;
class EngineVuMeter;
class ControlPotmeter;
class ControlPushButton;

/**
  *@author Tue and Ken Haste Andersen
  */

class EngineMaster : public EngineObject
{
public:
    EngineMaster(EngineBuffer *buffer1, EngineBuffer *buffer2,
                 EngineChannel *, EngineChannel *, EngineFlanger *, const char *group);
    ~EngineMaster();
    /** Reconfigures the EngineBufferScaleSRC objects with the sound quality written in the config database */
    void setPitchIndpTimeStretch(bool b);
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    EngineBuffer *buffer1, *buffer2;
    EngineChannel *channel1, *channel2;
    EngineVolume *volume, *head_volume; 
    EngineClipping *clipping, *head_clipping;
    EngineFlanger *flanger;
    EngineVuMeter *vumeter;

    ControlPotmeter *crossfader, *head_mix, *m_pBalance;
    ControlPushButton *pfl1, *pfl2, *flanger1, *flanger2;
    CSAMPLE *m_pMaster, *m_pHead, *m_pTemp1, *m_pTemp2;
    bool master1, master2;
};

#endif
