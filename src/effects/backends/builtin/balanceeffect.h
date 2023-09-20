#pragma once

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterlinkwitzriley4.h"
#include "util/memory.h"
#include "util/samplebuffer.h"

class BalanceGroupState : public EffectState {
  public:
    BalanceGroupState(const mixxx::EngineParameters& engineParameters);
    ~BalanceGroupState() override = default;

    void setFilters(mixxx::audio::SampleRate sampleRate, double freq);

    std::unique_ptr<EngineFilterLinkwitzRiley4Low> m_low;
    std::unique_ptr<EngineFilterLinkwitzRiley4High> m_high;

    mixxx::SampleBuffer m_pHighBuf;

    mixxx::audio::SampleRate m_oldSampleRate;
    double m_freq;

    CSAMPLE m_oldBalance;
    CSAMPLE m_oldMidSide;
};

class BalanceEffect : public EffectProcessorImpl<BalanceGroupState> {
  public:
    BalanceEffect() = default;
    ~BalanceEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            BalanceGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pBalanceParameter;
    EngineEffectParameterPointer m_pMidSideParameter;
    EngineEffectParameterPointer m_pBypassFreqParameter;

    DISALLOW_COPY_AND_ASSIGN(BalanceEffect);
};
