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

EngineVinylSoundEmu::EngineVinylSoundEmu(QString group)
        : m_dSpeed(0.0),
          m_dOldSpeed(0.0),
          m_iNoisePos(0) {
    Q_UNUSED(group);
    // Generate dither values. When engine samples used to be within [SAMPLE_MIN,
    // SAMPLE_MAX] dithering values were in the range [-0.5, 0.5]. Now that we
    // normalize engine samples to the range [-1.0, 1.0] we divide by SAMPLE_MAX
    // to preserve the previous behavior.
    for (int i = 0; i < NOISE_BUFFER_SIZE; ++i) {
        m_fNoise[i] = (static_cast<CSAMPLE>(rand() % RAND_MAX) / RAND_MAX - 0.5) / SAMPLE_MAX;
    }
    SampleUtil::applyGain(m_crossfadeBuffer, 0, MAX_BUFFER_LEN);
}

EngineVinylSoundEmu::~EngineVinylSoundEmu() {
}

void EngineVinylSoundEmu::setSpeed(double speed) {
    m_dSpeed = speed;
}

void EngineVinylSoundEmu::process(CSAMPLE* pInOut, const int iBufferSize) {
    const double thresholdSpeed = 0.070; //Scale volume if playback speed is below 7%.
    const double ditherSpeed = 0.85; //Dither if playback speed is below 85%.

    const bool wasDithering = m_dOldSpeed < ditherSpeed;
    const CSAMPLE prevQuieting =
            fabs(m_dOldSpeed) < thresholdSpeed ? fabs(m_dOldSpeed) / thresholdSpeed : 1.;
    const bool isDithering = m_dSpeed < ditherSpeed;
    const CSAMPLE curQuieting =
            fabs(m_dSpeed) < thresholdSpeed ? fabs(m_dSpeed) / thresholdSpeed : 1.;

    // If necessary, process with the old parameters and crossfade.
    if (wasDithering != isDithering || prevQuieting != curQuieting) {
        for (int i = 0; i < iBufferSize; i += 2) {
            CSAMPLE dither = 0;
            if (wasDithering) {
                dither = m_fNoise[m_iNoisePos];
                m_iNoisePos = (m_iNoisePos + 1) % NOISE_BUFFER_SIZE;
            }
            m_crossfadeBuffer[i] = prevQuieting * pInOut[i] + dither;
            m_crossfadeBuffer[i+1] = prevQuieting * pInOut[i+1] + dither;
        }
    }

    // First process with the current parameter state.
    for (int i = 0; i < iBufferSize; i += 2) {
        CSAMPLE dither = 0;
        if (isDithering) {
            dither = m_fNoise[m_iNoisePos];
            m_iNoisePos = (m_iNoisePos + 1) % NOISE_BUFFER_SIZE;
        }
        pInOut[i] = curQuieting * pInOut[i] + dither;
        pInOut[i+1] = curQuieting * pInOut[i+1] + dither;
    }

    if (wasDithering != isDithering || prevQuieting != curQuieting) {
        SampleUtil::linearCrossfadeBuffers(pInOut, m_crossfadeBuffer, pInOut, iBufferSize);
    }

    m_dOldSpeed = m_dSpeed;
}
