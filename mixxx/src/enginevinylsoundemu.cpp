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

EngineVinylSoundEmu::EngineVinylSoundEmu(ConfigObject<ConfigValue> *pConfig, const char *group)
{
    m_pConfig = pConfig;
    m_pRateEngine = ControlObject::getControl(ConfigKey(group, "rateEngine"));
    m_fSpeed = 0.0f;
    m_fGainFactor = 1.0f;
}

EngineVinylSoundEmu::~EngineVinylSoundEmu()
{

}

void EngineVinylSoundEmu::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    CSAMPLE *pOutput = (CSAMPLE *)pOut;
    m_fSpeed = (float)m_pRateEngine->get();
    //qDebug() << m_pRateEngine->get();
    
    if (fabs(m_fSpeed) < 0.10f)
    {
        m_fGainFactor = 0.0f; //Helps kill the massive aliasing that occurs at super slow speeds
    }
    if (fabs(m_fSpeed) < 0.50f && m_fSpeed != 0.0f) //Change the volume based on the playback speed.
    {
        m_fGainFactor = fabs(m_fSpeed)/0.50f * 1.10f;
    }
    else
    {
        m_fGainFactor = 1.0f;
    }
    //qDebug() << "gf: " << m_fGainFactor;

    //Apply whatever gain we calculated.
    for (int i=0; i<iBufferSize; ++i)
    {
        pOutput[i] = pOutput[i] * m_fGainFactor;
    } 
    
}

