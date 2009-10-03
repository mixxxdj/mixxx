/***************************************************************************
                          enginetemporal.cpp  -  description
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

#include "enginetemporal.h"
#include "enginebuffer.h"
#include "controlobject.h"
#include "controlpotmeter.h"
#include "controlttrotary.h"
#include "configobject.h"
#include "defs.h"
#include "mathstuff.h"

EngineTemporal::EngineTemporal(const char * group, EngineObject * pEffect)
{
    m_pEffect = pEffect;
    m_pEngineBuffer = 0;

    m_pControlShape = new ControlPotmeter(ConfigKey(group, "temporalShape"), 0., 1.);
    m_pControlShapeRate = new ControlTTRotary(ConfigKey(group, "temporalShapeRate"));
    m_pControlShape->set(1./(float)kiTempWindowNo);
    connect(m_pControlShapeRate, SIGNAL(valueChanged(double)), this, SLOT(slotShapeUpdate(double)));

    m_pControlPhase = new ControlPotmeter(ConfigKey(group, "temporalPhase"), 0., 1.);
    m_pControlPhaseRate = new ControlTTRotary(ConfigKey(group, "temporalPhaseRate"));
    m_pControlPhase->set(0.);
    connect(m_pControlPhaseRate, SIGNAL(valueChanged(double)), this, SLOT(slotPhaseUpdate(double)));

    m_pControlBeatFirst = new ControlObject(ConfigKey(group, "temporalBeatFirst"));

    m_pTemp = new CSAMPLE[MAX_BUFFER_LEN];
}

EngineTemporal::~EngineTemporal()
{
    delete m_pControlPhase;
    delete m_pControlShape;
    delete m_pControlShapeRate;
    delete m_pControlBeatFirst;
    delete [] m_pTemp;
}

void EngineTemporal::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    pOut = pIn;
    return;

    float * pOutput = (float *)pOut;

    m_pEffect->process(pIn, m_pTemp, iBufferSize);

    //m_pEffect->process(pIn, pOut, iBufferSize);
    //return;

    float rate = 44100.; //(float)getPlaySrate();

    //if (m_pEngineBuffer)
    //    rate /= (m_pEngineBuffer->getRate()+1);

    float fBpm = 60.;
    float fAbsPos = 0.;

    if (m_pEngineBuffer)
    {
        fBpm = m_pEngineBuffer->getBpm();
        fAbsPos = m_pEngineBuffer->getAbsPlaypos();
    }
    float fPhaseInc = (fBpm/(60.*2.))/rate;
    while (fPhaseInc>1.)
        fPhaseInc--;
    float fPhase = fAbsPos*fPhaseInc+m_pControlPhase->get();

    for (int i=0; i<iBufferSize; ++i)
    {
        while (fPhase>1.)
            fPhase--;

        //float temp = fPhase-(floor)(int)fPhase;
        float f = temporalWindow(m_pControlShape->get(), fPhase);

        pOutput[i] = (f*m_pTemp[i]);

        fPhase += fPhaseInc;
    }
}

void EngineTemporal::setEngineBuffer(EngineBuffer * pEngineBuffer)
{
    m_pEngineBuffer = pEngineBuffer;

    Q_ASSERT(m_pEngineBuffer);
}

float EngineTemporal::temporalWindow(float fShape, float fPos)
{
    return 1.;

    Q_ASSERT(fPos>=0.   && fPos<1.);
    Q_ASSERT(fShape>=0. && fShape<=1.);

    while (fPos>=1.)
        fPos -= 1.;

    //fShape *= (float)kiTempWindowNo;
    float fShapePos = fShape*((float)kiTempWindowNo/2.);

    // Find indexes of two influential windows for the given shape value
    int iWndIdx1, iWndIdx2;
    iWndIdx1 = (int)(fShapePos*2.);
    iWndIdx2 = (iWndIdx1-1+kiTempWindowNo)%kiTempWindowNo;

    //fShape *= 2.;
    //while (fShape>=1.)
    //    fShape -= 1.;

    float fIdx = fPos*(float)kiTempWindowLength;
    int iIdx1 = (int)(fIdx);
    int iIdx2 = iIdx1+1;
    float fFraction = fIdx-(float)iIdx1;

    float fReturnValue = 0.;
    fReturnValue += (fShape)   *((1.-fFraction)*kfTempWindows[(iWndIdx1*kiTempWindowLength+iIdx1)%(kiTempWindowLength*kiTempWindowNo)]+fFraction*kfTempWindows[(iWndIdx1*kiTempWindowLength+iIdx2)%(kiTempWindowLength*kiTempWindowNo)]);
    fReturnValue += (1.-fShape)*((1.-fFraction)*kfTempWindows[(iWndIdx2*kiTempWindowLength+iIdx1)%(kiTempWindowLength*kiTempWindowNo)]+fFraction*kfTempWindows[(iWndIdx2*kiTempWindowLength+iIdx2)%(kiTempWindowLength*kiTempWindowNo)]);

    //fReturnValue = fmax(-0.99, fmin(0.99, fReturnValue));

    return fReturnValue;
}

void EngineTemporal::slotShapeUpdate(double v)
{
    float fShape = m_pControlShape->get()*4.;
    fShape += (v/100.);
    while (fShape>=1.)
        fShape-=1.;
    while (fShape<0.)
        fShape+=1.;

    m_pControlShape->set(fShape);
}

void EngineTemporal::slotPhaseUpdate(double v)
{
    float fPhase = m_pControlPhase->get()*4.;
    fPhase += (v/100.);
    while (fPhase>1.)
        fPhase-=1.;
    while (fPhase<0.)
        fPhase+=1.;

    m_pControlPhase->set(fPhase);
}
