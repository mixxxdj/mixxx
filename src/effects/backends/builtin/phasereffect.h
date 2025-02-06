#pragma once

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/sample.h"
#include "util/types.h"

#define MAXSTAGES 12

class PhaserGroupState final : public EffectState {
  public:
    PhaserGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        clear();
    }
    ~PhaserGroupState() override = default;

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
    PhaserEffect() = default;
    ~PhaserEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            PhaserGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pStagesParameter;
    EngineEffectParameterPointer m_pLFOPeriodParameter;
    EngineEffectParameterPointer m_pDepthParameter;
    EngineEffectParameterPointer m_pFeedbackParameter;
    EngineEffectParameterPointer m_pRangeParameter;
    EngineEffectParameterPointer m_pTripletParameter;
    EngineEffectParameterPointer m_pStereoParameter;

    //Passing the sample through a series of allpass filters
    inline CSAMPLE processSample(CSAMPLE input,
            CSAMPLE* oldIn,
            CSAMPLE* oldOut,
            CSAMPLE mainCoef,
            int stages) {
        for (int j = 0; j < stages; j++) {
            oldOut[j] = (mainCoef * input) + (mainCoef * oldOut[j]) - oldIn[j];
            oldIn[j] = input;
            input = oldOut[j];
        }
        return input;
    }

    DISALLOW_COPY_AND_ASSIGN(PhaserEffect);
};
