#ifndef GATEREFFECT_H
#define GATEREFFECT_H

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"


struct GaterGroupState {
    enum State {IDLE, ATTACK, HOLD, RELEASE} state;
    double gain;
    unsigned int currentFrame;
    unsigned int holdCounter;
};

class GaterEffect : public PerChannelEffectProcessor<GaterGroupState> {
  public:
    GaterEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~GaterEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        GaterGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pRateParameter;
    EngineEffectParameter* m_pShapeParameter;
    EngineEffectParameter* m_pQuantizeParameter;
    EngineEffectParameter* m_pTripletParameter;
    EngineEffectParameter* m_pPhaseParameter;

    DISALLOW_COPY_AND_ASSIGN(GaterEffect);
};

#endif /* GATEREFFECT_H */
