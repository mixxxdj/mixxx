/***************************************************************************
                          enginedelay.h  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
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

#ifndef ENGINEDELAY_H
#define ENGINEDELAY_H

#include "engineobject.h"

class ControlPotmeter;

const int kiMaxDelay = 20000; 

class EngineDelay : public EngineObject
{
public:
    EngineDelay(const char *group);
    ~EngineDelay();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    ControlPotmeter *m_pPotmeter;
    CSAMPLE *m_pDelayBuffer;
    int m_iDelay, m_iDelayPos;
};

#endif
