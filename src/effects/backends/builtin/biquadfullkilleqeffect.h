#pragma once

#include "control/controlproxy.h"
#include "effects/backends/builtin/lvmixeqbase.h"
#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterbessel4.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "engine/filters/enginefilterdelay.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/memory.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class BiquadFullKillEQEffectGroupState : public EffectState {
  public:
    BiquadFullKillEQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~BiquadFullKillEQEffectGroupState() override = default;

    void setFilters(
            mixxx::audio::SampleRate sampleRate,
            double lowFreqCorner,
            double highFreqCorner);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_midBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highBoost;
    std::unique_ptr<EngineFilterBiquad1LowShelving> m_lowKill;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_midKill;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_highKill;
    std::unique_ptr<LVMixEQEffectGroupState<EngineFilterBessel4Low>> m_lvMixIso;

    mixxx::SampleBuffer m_pLowBuf;
    mixxx::SampleBuffer m_pBandBuf;
    mixxx::SampleBuffer m_pHighBuf;
    mixxx::SampleBuffer m_tempBuf;

    double m_oldLowBoost;
    double m_oldMidBoost;
    double m_oldHighBoost;
    double m_oldLowKill;
    double m_oldMidKill;
    double m_oldHighKill;
    double m_oldLow;
    double m_oldMid;
    double m_oldHigh;

    double m_loFreqCorner;
    double m_highFreqCorner;

    SINT m_rampHoldOff;
    SINT m_groupDelay;

    mixxx::audio::SampleRate m_oldSampleRate;
};

class BiquadFullKillEQEffect : public EffectProcessorImpl<BiquadFullKillEQEffectGroupState> {
  public:
    BiquadFullKillEQEffect();
    ~BiquadFullKillEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            BiquadFullKillEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

    void setFilters(mixxx::audio::SampleRate sampleRate,
            double lowFreqCorner,
            double highFreqCorner);

  private:
    BiquadFullKillEQEffect(const BiquadFullKillEQEffect&) = delete;
    void operator=(const BiquadFullKillEQEffect&) = delete;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPotLow;
    EngineEffectParameterPointer m_pPotMid;
    EngineEffectParameterPointer m_pPotHigh;

    EngineEffectParameterPointer m_pKillLow;
    EngineEffectParameterPointer m_pKillMid;
    EngineEffectParameterPointer m_pKillHigh;

    std::unique_ptr<ControlProxy> m_pLoFreqCorner;
    std::unique_ptr<ControlProxy> m_pHiFreqCorner;
};
