#pragma once

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterlinkwitzriley4.h"
#include "util/samplebuffer.h"
#include "util/memory.h"

class BalanceGroupState : public EffectState {
  public:
    BalanceGroupState(const mixxx::EngineParameters& bufferParameters);
    ~BalanceGroupState();

    void setFilters(int sampleRate, double freq);

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
    BalanceEffect(EngineEffect* pEffect);
    virtual ~BalanceEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(const ChannelHandle& handle,
                        BalanceGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:

    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pBalanceParameter;
    EngineEffectParameter* m_pMidSideParameter;
    EngineEffectParameter* m_pBypassFreqParameter;

    DISALLOW_COPY_AND_ASSIGN(BalanceEffect);
};
