/***************************************************************************
                          enginepregain.cpp  -  description
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

#include <QtDebug>

#include "engine/enginepregain.h"
#include "controlaudiotaperpot.h"
#include "controlpotmeter.h"
#include "controlpushbutton.h"
#include "configobject.h"
#include "controlobject.h"
#include "util/math.h"
#include "sampleutil.h"

ControlPotmeter* EnginePregain::s_pReplayGainBoost = NULL;
ControlObject* EnginePregain::s_pEnableReplayGain = NULL;

/*----------------------------------------------------------------
   A pregaincontrol is ... a pregain.
   ----------------------------------------------------------------*/
EnginePregain::EnginePregain(QString group)
{
    m_pPotmeterPregain = new ControlAudioTaperPot(ConfigKey(group, "pregain"), -12, +12, 0.5);
    //Replay Gain things
    m_pCOReplayGain = new ControlObject(ConfigKey(group, "replaygain"));
    m_pTotalGain = new ControlObject(ConfigKey(group, "total_gain"));
    m_pPassthroughEnabled = ControlObject::getControl(ConfigKey(group, "passthrough"));

    if (s_pReplayGainBoost == NULL) {
        s_pReplayGainBoost = new ControlPotmeter(ConfigKey("[ReplayGain]", "ReplayGainBoost"), -6, 15);
        s_pEnableReplayGain = new ControlObject(ConfigKey("[ReplayGain]", "ReplayGainEnabled"));
    }
    m_bSmoothFade = false;

    m_fPrevGain = 1.0;
}

EnginePregain::~EnginePregain()
{
    delete m_pPotmeterPregain;
    delete m_pCOReplayGain;
    delete m_pTotalGain;

    delete s_pEnableReplayGain;
    s_pEnableReplayGain = NULL;
    delete s_pReplayGainBoost;
    s_pReplayGainBoost = NULL;
}

void EnginePregain::process(CSAMPLE* pInOut, const int iBufferSize) {
    const float fReplayGain = m_pCOReplayGain->get();
    float fReplayGainCorrection;
    if (m_pPassthroughEnabled->toBool()) {
        // Override replaygain value if passing through
        // TODO(XXX): consider a good default.
        // Do we expect an replaygain leveled input or
        // Normalized to 1 input?
        fReplayGainCorrection = 1; // We expect a replaygain leveled input
    } else if (!s_pEnableReplayGain->toBool() || fReplayGain == 0) {
        // use predicted replaygain
        fReplayGainCorrection = 0.5;
        // We prepare for smoothfading to ReplayGain suggested gain
        // if ReplayGain value changes or ReplayGain is enabled
        m_bSmoothFade = true;
        m_timer.restart();
    } else {
        // Here is the point, when ReplayGain Analyser takes its action,
        // suggested gain changes from 0 to a nonzero value
        // We want to smoothly fade to this last.
        // Anyway we have some the problem that code cannot block the
        // full process for one second.
        // So we need to alter gain each time ::process is called.

        const float fullReplayGainBoost = fReplayGain *
                (float)s_pReplayGainBoost->get();

        // This means that a ReplayGain value has been calculated after the
        // track has been loaded
        const double kFadeSeconds = 1.0;

        if (m_bSmoothFade) {
            double seconds = static_cast<double>(m_timer.elapsed()) / 1e9;
            if (seconds < kFadeSeconds) {
                // Fade smoothly
                double fadeFrac = seconds / kFadeSeconds;
                fReplayGainCorrection = m_fPrevGain * (1.0 - fadeFrac) +
                        fadeFrac * fullReplayGainBoost;
            } else {
                m_bSmoothFade = false;
                fReplayGainCorrection = fullReplayGainBoost;
            }
        } else {
            // Passing a user defined boost
            fReplayGainCorrection = fullReplayGainBoost;
        }
    }

    // Clamp gain to within [0, 10.0] to prevent insane gains. This can happen
    // (some corrupt files get really high replay gain values).
    // 10 allows a maximum replay Gain Boost * calculated replay gain of ~2
    float totalGain = (float)m_pPotmeterPregain->get() *
            math_clamp(fReplayGainCorrection, 0.0f, 10.0f);

    m_pTotalGain->set(totalGain);

    if (totalGain != m_fPrevGain) {
        // Prevent sound wave discontinuities by interpolating from old to new gain.
        SampleUtil::applyRampingGain(pInOut, m_fPrevGain, totalGain, iBufferSize);
        m_fPrevGain = totalGain;
    } else {
        // SampleUtil deals with aliased buffers and gains of 1 or 0.
        SampleUtil::applyGain(pInOut, totalGain, iBufferSize);
    }
}
