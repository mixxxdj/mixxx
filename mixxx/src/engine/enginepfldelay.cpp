/***************************************************************************
                          enginepfldelay.cpp  -  description
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

#include "enginepfldelay.h"
#include "controlpotmeter.h"
#include "sampleutil.h"

/*----------------------------------------------------------------

   ----------------------------------------------------------------*/
EnginePflDelay::EnginePflDelay()
{
    m_pDelayBuffer = new CSAMPLE[kiMaxDelay];
    //make sure it's zeroed out
    SampleUtil::applyGain(m_pDelayBuffer, 0.0f, kiMaxDelay);
    m_iDelayPos = 0;
    
    m_pDelayPot = new ControlPotmeter(ConfigKey("[Master]", "pfl_delay"), 0, 1);
    connect(m_pDelayPot, SIGNAL(valueChanged(double)),
            this, SLOT(slotDelayChanged(double)),
            Qt::DirectConnection);
            
    m_iDelay = 0;
}

EnginePflDelay::~EnginePflDelay()
{
    delete [] m_pDelayBuffer;
}

void EnginePflDelay::slotDelayChanged(double new_delay)
{
    //if we actually pick a value of kiMaxDelay, it wraps around to zero
    m_iDelay = (int)(new_delay * (kiMaxDelay-2));
    if (m_iDelay % 2)
        m_iDelay++;
    //zero out the buffer to prevent noise
    SampleUtil::applyGain(m_pDelayBuffer, 0.0f, kiMaxDelay);
}

void EnginePflDelay::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    int iDelaySourcePos = (m_iDelayPos+kiMaxDelay-m_iDelay)%kiMaxDelay;
    CSAMPLE * pOutput = (CSAMPLE *)pOut;

    Q_ASSERT(iDelaySourcePos>=0);
    Q_ASSERT(iDelaySourcePos<=kiMaxDelay);

    for (int i=0; i<iBufferSize; ++i)
    {
        // put sample into delay buffer:
        m_pDelayBuffer[m_iDelayPos] = pIn[i];
        m_iDelayPos = (m_iDelayPos+1)%kiMaxDelay;

        // Take delayed sample from delay buffer and copy it to dest buffer:
        pOutput[i] = m_pDelayBuffer[iDelaySourcePos];
        iDelaySourcePos = (iDelaySourcePos+1)%kiMaxDelay;
    }
}

