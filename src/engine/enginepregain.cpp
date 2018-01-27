#include "engine/enginepregain.h"

#include <QtDebug>

#include "preferences/usersettings.h"
#include "control/controlaudiotaperpot.h"
#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "control/controlpushbutton.h"
#include "util/math.h"
#include "util/sample.h"

namespace {
    constexpr double kSpeedGainMultiplyer = 4.0; // Bends the speed to gain curve for a natural vinyl sound
    constexpr double kMaxTotalGainBySpeed = 0.9; // -1 dB to not risk any clipping even for lossy track that may
                                                 // have samples above 1.0
    const double kSpeedOneDiv = log10((1 * kSpeedGainMultiplyer) + 1); // value to normalize gain to 1 at speed one
} // anonymous namespace

ControlPotmeter* EnginePregain::s_pReplayGainBoost = NULL;
ControlPotmeter* EnginePregain::s_pDefaultBoost = NULL;
ControlObject* EnginePregain::s_pEnableReplayGain = NULL;

EnginePregain::EnginePregain(QString group)
        : m_dSpeed(1.0),
          m_dOldSpeed(1.0),
          m_dNonScratchSpeed(1.0),
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

void EnginePregain::setSpeedAndScratching(double speed, bool scratching) {
    m_dOldSpeed = m_dSpeed;
    m_dSpeed = speed;
    if (!scratching) {
        m_dNonScratchSpeed = speed;
    }
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
        // Here is the point, when ReplayGain Analyzer takes its action,
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
            double seconds = m_timer.elapsed().toDoubleSeconds();
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
    CSAMPLE_GAIN totalGain = (CSAMPLE_GAIN)m_pPotmeterPregain->get() *
            math_clamp(fReplayGainCorrection, 0.0f, 10.0f);

    m_pTotalGain->set(totalGain);

    // Vinylsoundemu:
    // Apply Gain change depending on the speed.
    // We have measured -Inf dB at x0, -6 dB at x0.3, 0 dB at x1 and 3.5 dB
    // at 2.5 using a real vinyl.
    // x5 is the maximum physically speed before the needle is starting to
    // loose contact to teh vinyl.
    // So we apply a curve here that emulates the gain change up to x 2.5 natural
    // to 3.5 dB and then limits the gain towards 5.5 dB at x5.
    // Since the additional gain will lead to undesired clipping,
    // we do not add more gain then we found in the original track.
    // This compensates a negative ReplayGain or PreGain setting.

    double speedGain = log10((fabs(m_dSpeed) * kSpeedGainMultiplyer) + 1) / kSpeedOneDiv;
    // Limit speed Gain to 0 dB if totalGain is already > 0.9 or Limit the
    // resulting totalGain to 0.9 for all other cases. This should avoid clipping even
    // if the source track has some samples above 1.0 due to lossy codecs.
    if (totalGain > kMaxTotalGainBySpeed) {
        speedGain = math_min(1.0, speedGain);
    } else {
        speedGain = math_min(kMaxTotalGainBySpeed / totalGain, speedGain);
    }
    totalGain *= speedGain;

    if ((m_dSpeed * m_dOldSpeed < 0) && m_scratching) {
        // direction changed, go though zero if scratching
        SampleUtil::applyRampingGain(&pInOut[0], m_fPrevGain, 0, iBufferSize / 2);
        SampleUtil::applyRampingGain(&pInOut[iBufferSize / 2], 0, totalGain, iBufferSize / 2);
    } else if (totalGain != m_fPrevGain) {
        // Prevent sound wave discontinuities by interpolating from old to new gain.
        SampleUtil::applyRampingGain(pInOut, m_fPrevGain, totalGain, iBufferSize);
    } else {
        // SampleUtil deals with aliased buffers and gains of 1 or 0.
        SampleUtil::applyGain(pInOut, totalGain, iBufferSize);
    }
    m_fPrevGain = totalGain;
}

void EnginePregain::collectFeatures(GroupFeatureState* pGroupFeatures) const {
    pGroupFeatures->gain = m_pPotmeterPregain->get();
    pGroupFeatures->has_gain = true;
}

