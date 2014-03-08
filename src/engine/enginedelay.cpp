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

EngineDelay::EngineDelay(const char* group, bool head)
        : m_iDelayPos(0),
          m_iDelay(0) {
    m_pDelayBuffer = new CSAMPLE[kiMaxDelay];
    memset(m_pDelayBuffer, 0, kiMaxDelay * sizeof(CSAMPLE));
    if (head) {
        m_pDelayPot = new ControlPotmeter(ConfigKey(group, "headDelay"), 0, kiMaxDelay);
    } else {
        m_pDelayPot = new ControlPotmeter(ConfigKey(group, "delay"), 0, kiMaxDelay);
    }
    connect(m_pDelayPot, SIGNAL(valueChanged(double)), this,
            SLOT(slotDelayChanged(double)), Qt::DirectConnection);
}

EngineDelay::~EngineDelay() {
    delete [] m_pDelayBuffer;
    delete m_pDelayPot;
}

void EngineDelay::slotDelayChanged(double new_delay) {
    // if we actually pick a value of kiMaxDelay, it wraps around to zero
    m_iDelay = (int)(new_delay * (kiMaxDelay - 2));
    if (m_iDelay % 2) {
        m_iDelay++;
    }
}


void EngineDelay::process(const CSAMPLE* pIn, CSAMPLE* pOutput, const int iBufferSize) {
    int iDelaySourcePos = (m_iDelayPos + kiMaxDelay - m_iDelay) % kiMaxDelay;

    Q_ASSERT(iDelaySourcePos >= 0);
    Q_ASSERT(iDelaySourcePos <= kiMaxDelay);

    for (int i = 0; i < iBufferSize; ++i) {
        // put sample into delay buffer:
        m_pDelayBuffer[m_iDelayPos] = pIn[i];
        m_iDelayPos = (m_iDelayPos + 1) % kiMaxDelay;

        // Take delayed sample from delay buffer and copy it to dest buffer:
        pOutput[i] = m_pDelayBuffer[iDelaySourcePos];
        iDelaySourcePos = (iDelaySourcePos + 1) % kiMaxDelay;
    }
}
