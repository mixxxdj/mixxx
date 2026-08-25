#pragma once

#include <memory>

#include "control/pollingcontrolproxy.h"
#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class TwoBandBiquadEQEffectGroupState final : public EffectState {
  public:
    TwoBandBiquadEQEffectGroupState(const mixxx::EngineParameters& engineParameters);
    ~TwoBandBiquadEQEffectGroupState() override = default;

    void setFilters(
            mixxx::audio::SampleRate sampleRate, double freqCorner);

    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_highBoost;
    std::unique_ptr<EngineFilterBiquad1Peaking> m_lowCut;
    std::unique_ptr<EngineFilterBiquad1HighShelving> m_highCut;
    mixxx::SampleBuffer m_tempBuf;
    double m_oldLowBoost;
    double m_oldHighBoost;
    double m_oldLowCut;
    double m_oldHighCut;

    double m_freqCorner;

    mixxx::audio::SampleRate m_oldSampleRate;
};

class TwoBandBiquadEQEffect : public EffectProcessorImpl<TwoBandBiquadEQEffectGroupState> {
  public:
    TwoBandBiquadEQEffect();
    ~TwoBandBiquadEQEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            TwoBandBiquadEQEffectGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

    void setFilters(mixxx::audio::SampleRate sampleRate,
            double lowFreqCorner,
            double highFreqCorner);

  private:
    TwoBandBiquadEQEffect(const TwoBandBiquadEQEffect&) = delete;
    void operator=(const TwoBandBiquadEQEffect&) = delete;

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pPotLow;
    EngineEffectParameterPointer m_pPotHigh;

    EngineEffectParameterPointer m_pKillLow;
    EngineEffectParameterPointer m_pKillHigh;

    PollingControlProxy m_pFreqCorner;
};
