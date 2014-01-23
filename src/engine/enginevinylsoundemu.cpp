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

#include "configobject.h"
#include "controlobject.h"
#include "engine/enginevinylsoundemu.h"
#include "sampleutil.h"

/** This class emulates the response of a vinyl record's audio to changes
 *   in speed. In practice, it quiets the audio during very slow playback.
 *   Dithering also helps mask the aliasing due to interpolation that occurs at
 *   these slow speeds.
 */

EngineVinylSoundEmu::EngineVinylSoundEmu(ConfigObject<ConfigValue>* pConfig,
                                         const char* group)
        : m_pConfig(pConfig),
          m_pRateEngine(ControlObject::getControl(
              ConfigKey(group, "rateEngine"))),
          m_dSpeed(0.0),
          m_dOldSpeed(0.0),
          m_iNoisePos(0) {
    // Generate dither values. When engine samples used to be within [SHRT_MIN,
    // SHRT_MAX] dithering values were in the range [-0.5, 0.5]. Now that we
    // normalize engine samples to the range [-1.0, 1.0] we divide by SHRT_MAX
    // to preserve the previous behavior.
    for (int i = 0; i < NOISE_BUFFER_SIZE; ++i) {
        m_fNoise[i] = (static_cast<CSAMPLE>(rand() % RAND_MAX) / RAND_MAX - 0.5) / SHRT_MAX;
    }
}

EngineVinylSoundEmu::~EngineVinylSoundEmu() {
}

void EngineVinylSoundEmu::process(const CSAMPLE* pIn, CSAMPLE* pOutput, const int iBufferSize) {
    m_dSpeed = m_pRateEngine->get();
    double rateFrac = 2.0 * (m_dSpeed - m_dOldSpeed) / iBufferSize;
    double curRate = m_dOldSpeed;

    const double thresholdSpeed = 0.070; //Scale volume if playback speed is below 7%.
    const double ditherSpeed = 0.85; //Dither if playback speed is below 85%.

    //iterate over old rate to new rate to prevent audible pops
    for (int i = 0; i < iBufferSize; i += 2) {
        CSAMPLE absCurRate = fabs(curRate);
        CSAMPLE dither = 0;
        if (absCurRate < ditherSpeed) {
            dither = m_fNoise[m_iNoisePos];
            m_iNoisePos = (m_iNoisePos + 1) % NOISE_BUFFER_SIZE;
        }

        if (absCurRate < thresholdSpeed) {
            CSAMPLE gainfrac = absCurRate / thresholdSpeed;
            pOutput[i] = gainfrac * pIn[i] + dither;
            pOutput[i+1] = gainfrac * pIn[i+1] + dither;
        } else {
           pOutput[i] = pIn[i] + dither;
           pOutput[i+1] = pIn[i+1] + dither;
        }

        curRate += rateFrac;
    }
    m_dOldSpeed = m_dSpeed;
}
