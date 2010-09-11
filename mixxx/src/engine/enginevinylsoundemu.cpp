/***************************************************************************
                          enginevinylsoundemu.cpp  -  description
                             -------------------
    copyright            : (C) 2007 by Albert Santoni
    email                : gamegod \a\t users.sf.net
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QtDebug>
#include <QtGlobal>
#include "configobject.h"
#include "controlobject.h"
#include "enginevinylsoundemu.h"


/** This class emulates the response of a vinyl record's audio to changes
 *   in speed. In practice, it quiets the audio during very slow playback.
 *   This also helps mask the aliasing due to interpolation that occurs at
 *   these slow speeds.
 */

EngineVinylSoundEmu::EngineVinylSoundEmu(ConfigObject<ConfigValue> * pConfig, const char * group)
{
    m_pConfig = pConfig;
    m_pRateEngine = ControlObject::getControl(ConfigKey(group, "rateEngine"));
    m_fAbsSpeed = 0.0f;
    m_fGainFactor = 1.0f;
}

EngineVinylSoundEmu::~EngineVinylSoundEmu()
{

}

void EngineVinylSoundEmu::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    m_fAbsSpeed = fabs((float)m_pRateEngine->get());
    //qDebug() << m_pRateEngine->get();
    
    const float thresholdSpeed = 0.070f; //Scale volume if playback speed is below 7%.
    if (m_fAbsSpeed < thresholdSpeed && m_fAbsSpeed > 0.0f) //Change the volume based on the playback speed.
    {
        //The numbers in this formula are important:
        //  - The "1 + ..." makes the minimum value of the parameter of log10
        //    be 1, which makes the gain 0.
        //  - The "* 9" makes the maximum value of the log10 become 10 (9 + 1 = 10)
        //    which gives a gain of 1
        //m_fGainFactor = log10(1 + m_fAbsSpeed/0.50f * 9);
        m_fGainFactor = m_fAbsSpeed/thresholdSpeed;
        //qDebug() << m_fGainFactor << m_fAbsSpeed;
    }
    else if (m_fAbsSpeed == 0.0f)
        m_fGainFactor = 0.0f; //Log blows up at 0 :)
    else 
    {
        m_fGainFactor = 1.0f;
    }

    //Apply whatever gain we calculated.
    for (int i=0; i < iBufferSize; i++)
    {
        pOutput[i] = pOutput[i] * m_fGainFactor;
    }

}

