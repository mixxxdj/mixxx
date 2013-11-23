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
#include "sampleutil.h"

/** This class emulates the response of a vinyl record's audio to changes
 *   in speed. In practice, it quiets the audio during very slow playback.
 *   Dithering also helps mask the aliasing due to interpolation that occurs at
 *   these slow speeds.
 */

EngineVinylSoundEmu::EngineVinylSoundEmu(ConfigObject<ConfigValue> * pConfig, const char * group)
{
    m_pConfig = pConfig;
    m_pRateEngine = ControlObject::getControl(ConfigKey(group, "rateEngine"));
    m_fSpeed = m_fOldSpeed = 0.0f;
    m_fGainFactor = 1.0f;
    m_iNoisePos = 0;

    for (int i=0; i<NOISE_BUFFER_SIZE; i++)
    {
        m_fNoise[i] = (float)(rand() % RAND_MAX) / RAND_MAX - 0.5;
    }
}

EngineVinylSoundEmu::~EngineVinylSoundEmu()
{

}

void EngineVinylSoundEmu::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    m_fSpeed = (float)m_pRateEngine->get();
    float rateFrac = 2 * (m_fSpeed - m_fOldSpeed) / (float)iBufferSize;
    float curRate = m_fOldSpeed;

    const float thresholdSpeed = 0.070f; //Scale volume if playback speed is below 7%.
    const float ditherSpeed = 0.85f; //Dither if playback speed is below 85%.

    //iterate over old rate to new rate to prevent audible pops
    for (int i=0; i<iBufferSize; i+=2)
    {
        float absCurRate = fabs(curRate);
        float dither = 0;
        if (absCurRate < ditherSpeed) {
            dither = m_fNoise[m_iNoisePos];
            m_iNoisePos = (m_iNoisePos + 1) % NOISE_BUFFER_SIZE;
        }

        if (absCurRate < thresholdSpeed) {
            float gainfrac = absCurRate / thresholdSpeed;
            pOutput[i] = gainfrac * (float)pIn[i] + dither;
            pOutput[i+1] = gainfrac * (float)pIn[i+1] + dither;
        }
        else
        {
           pOutput[i] = pIn[i] + dither;
           pOutput[i+1] = pIn[i+1] + dither;
        }

        curRate += rateFrac;
    }
    m_fOldSpeed = m_fSpeed;
}

