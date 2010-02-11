/***************************************************************************
                          enginetemporal.h  -  description
                             -------------------
    begin                : Tue Aug 31 2004
    copyright            : (C) 2002 by Tue Haste Andersen
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

/** BIG FAT WARNING : 7/4/2009 -- Mixxx hasn't used this in ages, we just keep
    it around for posterity. */

#ifndef ENGINETEMPORAL_H
#define ENGINETEMPORAL_H

#include "engineobject.h"

class ControlEngine;
class EngineBuffer;
class ControlObject;
class ControlPotmeter;
class ControlTTRotary;

static const int kiTempWindowLength = 11;
static const int kiTempWindowNo = 5;
//static const float kfTempWindows[kiTempWindowLength*kiTempWindowNo] = {1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0,
//                                                                        0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 0.8, 0.6, 0.4, 0.2, 0.0};

static const float kfTempWindows[kiTempWindowLength*kiTempWindowNo] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
                                                                       1.0f, 0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f,
                                                                       1.0f, 0.5f, 0.2f, 0.1f, 0.05f, 0.02f, 0.01f, 0.f, 0.f, 0.f, 0.f,
                                                                       1.0f, 0.1f, 0.05f, 0.01f, 0.001f, 0.f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                                                       0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};

/**
  *@author Tue Haste Andersen
  */
class EngineTemporal : public EngineObject  
{
    Q_OBJECT
public:
    EngineTemporal(const char *group, EngineObject *pEffect);
    ~EngineTemporal();

    void setEngineBuffer(EngineBuffer *pEngineBuffer);
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

    static float temporalWindow(float fShape, float fPos);
    
public slots:
    void slotShapeUpdate(double v);    
    void slotPhaseUpdate(double v);    
    
private:
    CSAMPLE *m_pTemp1, *m_pTemp2, *m_pTemp3;
    EngineObject *m_pEffect;
    EngineBuffer *m_pEngineBuffer;
    /** Shape and phase of temporal curve */
    ControlObject *m_pControlBeatFirst;
    ControlPotmeter *m_pControlShape, *m_pControlPhase;
    ControlTTRotary *m_pControlShapeRate, *m_pControlPhaseRate;
    float *m_pTemp;
};

#endif
