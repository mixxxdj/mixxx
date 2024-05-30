#pragma once

#include <memory>

#include "control/pollingcontrolproxy.h"
#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class ThreeBandBiquadEQEffectGroupState final : public EffectState {
  public:
    ThreeBandBiquadEQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~ThreeBandBiquadEQEffectGroupState() override = default;

    void setFilters(
            mixxx::audio::SampleRate sampleRate, double lowFreqCorner, double highFreqCorner);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_midBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowCut;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_midCut;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_highCut;
    mixxx::SampleBuffer m_tempBuf;
    double m_oldLowBoost;
    double m_oldMidBoost;
    double m_oldHighBoost;
    double m_oldLowCut;
    double m_oldMidCut;
    double m_oldHighCut;

    double m_loFreqCorner;
    double m_highFreqCorner;

    mixxx::audio::SampleRate m_oldSampleRate;
};

class ThreeBandBiquadEQEffect : public EffectProcessorImpl<ThreeBandBiquadEQEffectGroupState> {
  public:
    ThreeBandBiquadEQEffect();
    ~ThreeBandBiquadEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            ThreeBandBiquadEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

    void setFilters(mixxx::audio::SampleRate sampleRate,
            double lowFreqCorner,
            double highFreqCorner);

  private:
    ThreeBandBiquadEQEffect(const ThreeBandBiquadEQEffect&) = delete;
    void operator=(const ThreeBandBiquadEQEffect&) = delete;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPotLow;
    EngineEffectParameterPointer m_pPotMid;
    EngineEffectParameterPointer m_pPotHigh;

    EngineEffectParameterPointer m_pKillLow;
    EngineEffectParameterPointer m_pKillMid;
    EngineEffectParameterPointer m_pKillHigh;

    PollingControlProxy m_pLoFreqCorner;
    PollingControlProxy m_pHiFreqCorner;
};
