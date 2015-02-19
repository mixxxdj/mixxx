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
EnginePregain::EnginePregain(const StringAtom& group)
        : m_dSpeed(0),
          m_fPrevGain(1.0),
          m_bSmoothFade(false) {
    m_pPotmeterPregain = new ControlAudioTaperPot(ConfigKey(group, "pregain"), -12, +12, 0.5);
    //Replay Gain things
    m_pControlReplayGain = new ControlObject(ConfigKey(group, "replaygain"));
    m_pTotalGain = new ControlObject(ConfigKey(group, "total_gain"));
    m_pPassthroughEnabled = ControlObject::getControl(ConfigKey(group, "passthrough"));

    if (s_pReplayGainBoost == NULL) {
        s_pReplayGainBoost = new ControlPotmeter(ConfigKey("[ReplayGain]", "ReplayGainBoost"), -6, 15);
        s_pEnableReplayGain = new ControlObject(ConfigKey("[ReplayGain]", "ReplayGainEnabled"));
    }
}

EnginePregain::~EnginePregain() {
    delete m_pPotmeterPregain;
    delete m_pControlReplayGain;
    delete m_pTotalGain;

    delete s_pEnableReplayGain;
    s_pEnableReplayGain = NULL;
    delete s_pReplayGainBoost;
    s_pReplayGainBoost = NULL;
}

void EnginePregain::setSpeed(double speed) {
    m_dSpeed = speed;
}

void EnginePregain::process(CSAMPLE* pInOut, const int iBufferSize) {
    float fEnableReplayGain = s_pEnableReplayGain->get();
    const float fReplayGainBoost = s_pReplayGainBoost->get();
    float fGain = m_pPotmeterPregain->get();
    float fReplayGain = m_pControlReplayGain->get();
    float fReplayGainCorrection = 1;
    float fPassing = m_pPassthroughEnabled->get();

    // Override replaygain value if passing through
    if (fPassing > 0) {
        fReplayGain = 1.0;
    } else if (fReplayGain * fEnableReplayGain != 0) {
        // Here is the point, when ReplayGain Analyser takes its action, suggested gain changes from 0 to a nonzero value
        // We want to smoothly fade to this last.
        // Anyway we have some the problem that code cannot block the full process for one second.
        // So we need to alter gain each time ::process is called.

        const float fullReplayGainBoost = fReplayGain * fReplayGainBoost;

        // This means that a ReplayGain value has been calculated after the track has been loaded
        const double kFadeSeconds = 1.0;

        if (m_bSmoothFade) {
            double seconds = static_cast<double>(m_timer.elapsed()) / 1e9;
            if (seconds < kFadeSeconds) {
                // Fade smoothly
                double fadeFrac = seconds / kFadeSeconds;
                fReplayGainCorrection = (1.0 - fadeFrac) + fadeFrac * fullReplayGainBoost;
            } else {
                m_bSmoothFade = false;
                fReplayGainCorrection = fullReplayGainBoost;
            }
        } else {
            // Passing a user defined boost
            fReplayGainCorrection = fullReplayGainBoost;
        }
    } else if (fEnableReplayGain != 0) {
        // If track has not ReplayGain value and ReplayGain is enabled
        // we prepare for smoothfading to ReplayGain suggested gain
        m_bSmoothFade = true;
        m_timer.restart();
    }

    // Clamp gain to within [0, 10.0] to prevent insane gains. This can happen
    // (some corrupt files get really high replay gain values).
    // 10 allows a maximum replay Gain Boost * calculated replay gain of ~2
    fGain *= math_clamp(fReplayGainCorrection, 0.0f, 10.0f);

    m_pTotalGain->set(fGain);

    // Vinylsoundemu:
    // As the speed approaches zero, hearing small bursts of sound at full volume
    // is distracting and doesn't mimic the way that vinyl sounds when played slowly.
    // Instead, reduce gain to provide a soft rolloff.
    const float kThresholdSpeed = 0.070; // Scale volume if playback speed is below 7%.
    if (fabs(m_dSpeed) < kThresholdSpeed) {
        fGain *= fabs(m_dSpeed) / kThresholdSpeed;
    }

    if (fGain != m_fPrevGain) {
        // Prevent soundwave discontinuities by interpolating from old to new gain.
        SampleUtil::applyRampingGain(pInOut, m_fPrevGain, fGain, iBufferSize);
    } else {
        // SampleUtil deals with aliased buffers and gains of 1 or 0.
        SampleUtil::applyGain(pInOut, fGain, iBufferSize);
    }
    m_fPrevGain = fGain;
}
