
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
ControlPotmeter* EnginePregain::s_pDefaultBoost = NULL;
ControlObject* EnginePregain::s_pEnableReplayGain = NULL;

EnginePregain::EnginePregain(QString group)
        : m_dSpeed(1.0),
          m_dOldSpeed(1.0),
          m_scratching(false),
          m_fPrevGain(1.0),
          m_bSmoothFade(false) {
    m_pPotmeterPregain = new ControlAudioTaperPot(ConfigKey(group, "pregain"), -12, 12, 0.5);
    //Replay Gain things
    m_pCOReplayGain = new ControlObject(ConfigKey(group, "replaygain"));
    m_pTotalGain = new ControlObject(ConfigKey(group, "total_gain"));
    m_pPassthroughEnabled = ControlObject::getControl(ConfigKey(group, "passthrough"));

    if (s_pReplayGainBoost == NULL) {
        s_pReplayGainBoost = new ControlAudioTaperPot(ConfigKey("[ReplayGain]", "ReplayGainBoost"), -12, 12, 0.5);
        s_pDefaultBoost = new ControlAudioTaperPot(ConfigKey("[ReplayGain]", "DefaultBoost"), -12, 12, 0.5);
        s_pEnableReplayGain = new ControlObject(ConfigKey("[ReplayGain]", "ReplayGainEnabled"));
    }
}

EnginePregain::~EnginePregain() {
    delete m_pPotmeterPregain;
    delete m_pCOReplayGain;
    delete m_pTotalGain;

    delete s_pEnableReplayGain;
    s_pEnableReplayGain = NULL;
    delete s_pReplayGainBoost;
    s_pReplayGainBoost = NULL;
    delete s_pDefaultBoost;
    s_pDefaultBoost = NULL;
}

void EnginePregain::setSpeed(double speed) {
    m_dOldSpeed = m_dSpeed;
    m_dSpeed = speed;
}

void EnginePregain::setScratching(bool scratching) {
    m_scratching = scratching;
}

void EnginePregain::process(CSAMPLE* pInOut, const int iBufferSize) {
    const float fReplayGain = m_pCOReplayGain->get();
    float fReplayGainCorrection;
    if (!s_pEnableReplayGain->toBool() || m_pPassthroughEnabled->toBool()) {
        // Override replaygain value if passing through
        // TODO(XXX): consider a good default.
        // Do we expect an replaygain leveled input or
        // Normalized to 1 input?
        fReplayGainCorrection = 1; // We expect a replaygain leveled input
    } else if (fReplayGain == 0) {
        // use predicted replaygain
        fReplayGainCorrection = (float)s_pDefaultBoost->get();
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

    // Vinylsoundemu:
    // As the speed approaches zero, hearing small bursts of sound at full volume
    // is distracting and doesn't mimic the way that vinyl sounds when played slowly.
    // Instead, reduce gain to provide a soft rolloff.
    const float kThresholdSpeed = 0.070; // Scale volume if playback speed is below 7%.
    if (fabs(m_dSpeed) < kThresholdSpeed) {
        totalGain *= fabs(m_dSpeed) / kThresholdSpeed;
    }

    if ((m_dSpeed * m_dOldSpeed < 0) && m_scratching) {
        // direction changed, go though zero if scratching
        SampleUtil::applyRampingGain(&pInOut[0], m_fPrevGain, 0, iBufferSize / 2);
        SampleUtil::applyRampingGain(&pInOut[iBufferSize / 2], 0, totalGain, iBufferSize / 2);
    } else if (totalGain != m_fPrevGain) {
        // Prevent sound wave discontinuities by interpolating from old to new gain.
        SampleUtil::applyRampingGain(pInOut, m_fPrevGain, totalGain, iBufferSize);
        m_fPrevGain = totalGain;
    } else {
        // SampleUtil deals with aliased buffers and gains of 1 or 0.
        SampleUtil::applyGain(pInOut, totalGain, iBufferSize);
    }
}
