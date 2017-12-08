#ifndef TREMOLOEFFECT_H
#define TREMOLOEFFECT_H

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"


struct TremoloGroupState {
    double gain;
    unsigned int currentFrame;
    bool quantizeEnabled = false;
    bool tripletEnabled = false;
};

class TremoloEffect : public PerChannelEffectProcessor<TremoloGroupState> {
  public:
    TremoloEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    ~TremoloEffect() override;

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        TremoloGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pRateParameter;
    EngineEffectParameter* m_pShapeParameter;
    EngineEffectParameter* m_pSmoothParameter;
    EngineEffectParameter* m_pPhaseParameter;
    EngineEffectParameter* m_pQuantizeParameter;
    EngineEffectParameter* m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(TremoloEffect);
};

#endif /* TREMOLOEFFECT_H */
