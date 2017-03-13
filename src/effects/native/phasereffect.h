#ifndef PHASEREFFECT_H
#define PHASEREFFECT_H

#include "util.h"
#include "util/defs.h"
#include "util/types.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "effects/effectprocessor.h"
#include "sampleutil.h"
#include <QDebug>

#define MAXSTAGES 12

struct PhaserGroupState {
    PhaserGroupState() :
        leftPhase(0),
        rightPhase(0) { 
        SampleUtil::applyGain(oldInLeft, 0, MAXSTAGES);
        SampleUtil::applyGain(oldOutLeft, 0, MAXSTAGES);
        SampleUtil::applyGain(oldInRight, 0, MAXSTAGES);
        SampleUtil::applyGain(oldOutRight, 0, MAXSTAGES);
    }
    CSAMPLE oldInLeft[MAXSTAGES];
    CSAMPLE oldInRight[MAXSTAGES];
    CSAMPLE oldOutLeft[MAXSTAGES]; 
    CSAMPLE oldOutRight[MAXSTAGES];
    CSAMPLE leftPhase;
    CSAMPLE rightPhase;
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
    EngineEffectParameter* m_pDepthParameter; 
    EngineEffectParameter* m_pFeedbackParameter; 
    EngineEffectParameter* m_pRangeParameter; 
    EngineEffectParameter* m_pStereoParameter;

    //Passing the sample through a series of allpass filters
    inline CSAMPLE processSample(CSAMPLE input, CSAMPLE* oldIn, CSAMPLE* oldOut, 
                                 CSAMPLE mainCoef, int stages) { 
        for (int j = 0; j < stages; j++) {
            oldOut[j] = (mainCoef * input) + (mainCoef * oldOut[j]) - oldIn[j];
            oldIn[j] = input;
            input = oldOut[j];
        }
        return input;
    }

    DISALLOW_COPY_AND_ASSIGN(PhaserEffect);
};

#endif
