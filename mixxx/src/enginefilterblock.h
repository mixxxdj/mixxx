/***************************************************************************
                          enginefilterblock.h  -  description
                             -------------------
    begin                : Thu Apr 4 2002
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

#ifndef ENGINEFILTERBLOCK_H
#define ENGINEFILTERBLOCK_H

#include "engineobject.h"

class ControlLogpotmeter;
class ControlPushButton;

#define SIZE_NOISE_BUF 40
/**
  * Parallel processing of LP, BP and HP filters, and final mixing
  *
  *@author Tue and Ken Haste Andersen
  */

class EngineFilterBlock : public EngineObject  
{
public:
    EngineFilterBlock(const char *group);
    ~EngineFilterBlock();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
private:
    CSAMPLE *m_pTemp1, *m_pTemp2, *m_pTemp3;
    EngineObject *low, *band, *high;
    ControlLogpotmeter *filterpotLow, *filterpotMid, *filterpotHigh;
    ControlPushButton *filterKillLow, *filterKillMid, *filterKillHigh;
    CSAMPLE whiteNoiseBuf[SIZE_NOISE_BUF];
    int noiseCount;
};

#endif
