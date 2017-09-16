#ifndef PANEFFECT_H
#define BALANCEEFFECT_H

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterlinkwitzriley4.h"
#include "util/samplebuffer.h"
#include "util/memory.h"

// This effect does not need to store any state.
class BalanceGroupState final {
  public:
    BalanceGroupState();
    ~BalanceGroupState();

    void setFilters(int sampleRate, int freq);

    std::unique_ptr<EngineFilterLinkwitzRiley4Low> m_low;
    std::unique_ptr<EngineFilterLinkwitzRiley4High> m_high;

    SampleBuffer m_pHighBuf;

    unsigned int m_oldSampleRate;
    int m_freq;

    CSAMPLE m_oldBalance;
    CSAMPLE m_oldMidSide;
};

class BalanceEffect : public PerChannelEffectProcessor<BalanceGroupState> {
  public:
    BalanceEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~BalanceEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        BalanceGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
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

#endif /* BALANCEEFFECT_H */
