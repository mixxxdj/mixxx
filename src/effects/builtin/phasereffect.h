#pragma once

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

#define MAXSTAGES 12

class PhaserGroupState final : public EffectState {
  public:
    PhaserGroupState(const mixxx::EngineParameters& bufferParameters)
            : EffectState(bufferParameters) {
        clear();
    }

    void clear() {
        leftPhase = 0;
        rightPhase = 0;
        oldDepth = 0;
        SampleUtil::clear(oldInLeft, MAXSTAGES);
        SampleUtil::clear(oldOutLeft, MAXSTAGES);
        SampleUtil::clear(oldInRight, MAXSTAGES);
        SampleUtil::clear(oldOutRight, MAXSTAGES);
    }

    CSAMPLE oldInLeft[MAXSTAGES];
    CSAMPLE oldInRight[MAXSTAGES];
    CSAMPLE oldOutLeft[MAXSTAGES];
    CSAMPLE oldOutRight[MAXSTAGES];
    CSAMPLE leftPhase;
    CSAMPLE rightPhase;
    CSAMPLE_GAIN oldDepth;

};

class PhaserEffect : public EffectProcessorImpl<PhaserGroupState> {

  public:
    PhaserEffect(EngineEffect* pEffect);
    virtual ~PhaserEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        PhaserGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE* pOutput,
                        const mixxx::EngineParameters& bufferParameters,
                        const EffectEnableState enableState,
                        const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pStagesParameter;
    EngineEffectParameter* m_pLFOPeriodParameter;
    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pFeedbackParameter;
    EngineEffectParameter* m_pRangeParameter;
    EngineEffectParameter* m_pTripletParameter;
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
