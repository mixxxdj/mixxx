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
        time(0) {
        SampleUtil::applyGain(oldInLeft, 0, MAXSTAGES);
        SampleUtil::applyGain(oldOutLeft, 0, MAXSTAGES);
        SampleUtil::applyGain(oldInRight, 0, MAXSTAGES);
        SampleUtil::applyGain(oldOutRight, 0, MAXSTAGES);
        SampleUtil::applyGain(filterCoefLeft, 0, MAXSTAGES);
        SampleUtil::applyGain(filterCoefRight, 0, MAXSTAGES);
    }
    CSAMPLE oldInLeft[MAXSTAGES];
    CSAMPLE oldInRight[MAXSTAGES];
    CSAMPLE oldOutLeft[MAXSTAGES]; 
    CSAMPLE oldOutRight[MAXSTAGES];
    CSAMPLE filterCoefLeft[MAXSTAGES];
    CSAMPLE filterCoefRight[MAXSTAGES];
    int time;
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
    EngineEffectParameter* m_pFrequencyParameter;
    EngineEffectParameter* m_pDepthParameter; 
    EngineEffectParameter* m_pFeedback; 
    EngineEffectParameter* m_pSweepWidth; 

    DISALLOW_COPY_AND_ASSIGN(PhaserEffect);
};

#endif
