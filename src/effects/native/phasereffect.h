#ifndef PHASEREFFECT_H
#define PHASEREFFECT_H

#include "util.h"
#include "util/defs.h"
#include "util/types.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"

#define MAXSTAGES 24

struct PhaserGroupState {
    PhaserGroupState() :
        lfoShape(4.0),
        lfoSkipSamples(20) {
        SampleUtil::applyGain(oldLeft, 0, MAXSTAGES);
        SampleUtil::applyGain(oldRight, 0, MAXSTAGES);
    }
    CSAMPLE oldLeft[MAXSTAGES]; 
    CSAMPLE oldRight[MAXSTAGES];
    CSAMPLE lfoShape;
    int lfoSkipSamples;
};

class PhaserEffect : public PerChannelEffectProcessor<PhaserGroupState> {

  public:
    PhaserEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~PhaserEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        PhaserGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pStagesParameter;
    EngineEffectParameter* m_pLFOFrequencyParameter;
    EngineEffectParameter* m_pLFOStartPhaseParameter; 
    EngineEffectParameter* m_pDepthParameter; 



    DISALLOW_COPY_AND_ASSIGN(PhaserEffect);
};

#endif
