#pragma once

#include "engine/engineobject.h"
#include "control/controlobject.h"
#include "util/performancetimer.h"

class ControlAudioTaperPot;
class ControlPotmeter;
class ControlObject;

// The pregain control alters the volume of the track based on several inputs,
// including user pregain adjustment, ReplayGain value, and vinyl-like
// adjustments in volume relative to playback speed.
class EnginePregain : public EngineObject {
  public:
    EnginePregain(const QString& group);
    ~EnginePregain() override;

    // If the user is scratching and the record reverses direction, the volume
    // will be ramped to zero and back up again to mimic a vinyl scratch.
    // If the user is not scratching and the direction is reversed
    // (e.g. reverse button is pressed), the audio will be immediately
    // reversed without a ramp to zero.
    void setSpeedAndScratching(double speed, bool scratching);

    void process(CSAMPLE* pInOut, const int iBufferSize) override;

    void collectFeatures(GroupFeatureState* pGroupFeatures) const override;

  private:
    double m_dSpeed;
    double m_dOldSpeed;
    double m_dNonScratchSpeed;
    bool m_scratching;
    CSAMPLE_GAIN m_fPrevGain;
    ControlAudioTaperPot* m_pPotmeterPregain;
    ControlObject* m_pTotalGain;
    ControlObject* m_pCOReplayGain;
    ControlObject* m_pPassthroughEnabled;
    static ControlPotmeter* s_pReplayGainBoost;
    static ControlPotmeter* s_pDefaultBoost;
    static ControlObject* s_pEnableReplayGain;
    bool m_bSmoothFade;
    PerformanceTimer m_timer;
};
