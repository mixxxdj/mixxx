/***************************************************************************
                          enginepfldelay.h  -  description
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

#ifndef ENGINEPFLDELAY_H
#define ENGINEPFLDELAY_H

#include "engineobject.h"

class ControlPotmeter;

const int kiMaxDelay = MAX_BUFFER_LEN; 

class EnginePflDelay : public EngineObject
{
    Q_OBJECT
public:
    EnginePflDelay();
    ~EnginePflDelay();
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);
    
public slots:
    void slotDelayChanged(double);
    
private:
    ControlPotmeter *m_pDelayPot;
    CSAMPLE *m_pDelayBuffer;
    int m_iDelay, m_iDelayPos;
};

#endif
