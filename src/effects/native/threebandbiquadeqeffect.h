#ifndef THREEBANDBIQUADEQEFFECT_H
#define THREEBANDBIQUADEQEFFECT_H

#include "control/controlproxy.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

class ThreeBandBiquadEQEffectGroupState final {
  public:
    ThreeBandBiquadEQEffectGroupState();
    ~ThreeBandBiquadEQEffectGroupState();

    void setFilters(
            int sampleRate, double lowFreqCorner, double highFreqCorner);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_midBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowCut;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_midCut;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_highCut;
    SampleBuffer m_tempBuf;
    double m_oldLowBoost;
    double m_oldMidBoost;
    double m_oldHighBoost;
    double m_oldLowCut;
    double m_oldMidCut;
    double m_oldHighCut;

    double m_loFreqCorner;
    double m_highFreqCorner;

    unsigned int m_oldSampleRate;
};

class ThreeBandBiquadEQEffect : public PerChannelEffectProcessor<ThreeBandBiquadEQEffectGroupState> {
  public:
    ThreeBandBiquadEQEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    ~ThreeBandBiquadEQEffect() override;

    static QString getId();
    static EffectManifest getManifest();

    void setFilters(int sampleRate, double lowFreqCorner, double highFreqCorner);

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        ThreeBandBiquadEQEffectGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    ThreeBandBiquadEQEffect(const ThreeBandBiquadEQEffect&) = delete;
    void operator=(const ThreeBandBiquadEQEffect&) = delete;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pPotLow;
    EngineEffectParameter* m_pPotMid;
    EngineEffectParameter* m_pPotHigh;

    EngineEffectParameter* m_pKillLow;
    EngineEffectParameter* m_pKillMid;
    EngineEffectParameter* m_pKillHigh;

    std::unique_ptr<ControlProxy> m_pLoFreqCorner;
    std::unique_ptr<ControlProxy> m_pHiFreqCorner;
};

#endif // THREEBANDBIQUADEQEFFECT_H
