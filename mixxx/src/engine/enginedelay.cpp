/***************************************************************************
                          enginedelay.cpp  -  description
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

#include "enginedelay.h"
#include "controlpotmeter.h"

/*----------------------------------------------------------------

   ----------------------------------------------------------------*/
EngineDelay::EngineDelay(const char * group)
{
    m_pDelayBuffer = new CSAMPLE[kiMaxDelay];
    m_iDelayPos = 0;
    new ControlPotmeter(ConfigKey(group, "delay"), 0, kiMaxDelay);
}

EngineDelay::~EngineDelay()
{
    delete [] m_pDelayBuffer;
}

void EngineDelay::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
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

        // Take "old" sample from delay buffer and mix it with the source buffer:
        pOutput[i] = 0.5*(m_pDelayBuffer[iDelaySourcePos] + pIn[i]);
        iDelaySourcePos = (iDelaySourcePos+1)%kiMaxDelay;
    }
}

