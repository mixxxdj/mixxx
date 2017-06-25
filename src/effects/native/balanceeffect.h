#ifndef PANEFFECT_H
#define BALANCEEFFECT_H

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"

// This effect does not need to store any state.
struct BalanceGroupState {
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

    EngineEffectParameter* m_pLeftParameter;
    EngineEffectParameter* m_pRightParameter;
    EngineEffectParameter* m_pMidSideParameter;

    DISALLOW_COPY_AND_ASSIGN(BalanceEffect);
};

#endif /* BALANCEEFFECT_H */
