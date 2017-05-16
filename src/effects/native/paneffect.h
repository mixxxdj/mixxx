#ifndef PANEFFECT_H
#define PANEFFECT_H

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"

// This effect does not need to store any state.
struct PanGroupState {
};

class PanEffect : public PerChannelEffectProcessor<PanGroupState> {
  public:
    PanEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~PanEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        PanGroupState* pState,
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

    DISALLOW_COPY_AND_ASSIGN(PanEffect);
};

#endif /* PANEFFECT_H */
