#pragma once

#include <memory>

#include "control/pollingcontrolproxy.h"
#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class FourBandBiquadEQEffectGroupState final : public EffectState {
  public:
    FourBandBiquadEQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~FourBandBiquadEQEffectGroupState() override = default;

    void setFilters(mixxx::audio::SampleRate sampleRate,
            double lowFreqCorner,
            double midFreqCorner,
            double highFreqCorner);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowMidBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highMidBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowCut;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowMidCut;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highMidCut;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_highCut;
    mixxx::SampleBuffer m_tempBuf;
    double m_oldLowBoost;
    double m_oldLowMidBoost;
    double m_oldHighMidBoost;
    double m_oldHighBoost;
    double m_oldLowCut;
    double m_oldLowMidCut;
    double m_oldHighMidCut;
    double m_oldHighCut;

    double m_loFreqCorner;
    double m_midFreqCorner;
    double m_highFreqCorner;

    mixxx::audio::SampleRate m_oldSampleRate;
};

class FourBandBiquadEQEffect : public EffectProcessorImpl<FourBandBiquadEQEffectGroupState> {
  public:
    FourBandBiquadEQEffect();
    ~FourBandBiquadEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            FourBandBiquadEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

    void setFilters(mixxx::audio::SampleRate sampleRate,
            double lowFreqCorner,
            double midFreqCorner,
            double highFreqCorner);

  private:
    FourBandBiquadEQEffect(const FourBandBiquadEQEffect&) = delete;
    void operator=(const FourBandBiquadEQEffect&) = delete;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPotLow;
    EngineEffectParameterPointer m_pPotLowMid;
    EngineEffectParameterPointer m_pPotHighMid;
    EngineEffectParameterPointer m_pPotHigh;

    EngineEffectParameterPointer m_pKillLow;
    EngineEffectParameterPointer m_pKillLowMid;
    EngineEffectParameterPointer m_pKillHighMid;
    EngineEffectParameterPointer m_pKillHigh;

    PollingControlProxy m_pLoFreqCorner;
    PollingControlProxy m_pMidFreqCorner;
    PollingControlProxy m_pHiFreqCorner;
};
