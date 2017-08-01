#ifndef SEMIPARAMETRICEQ3KNOB_H
#define SEMIPARAMETRICEQ3KNOB_H

#include "control/controlbehavior.h"
#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/memory.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

struct SemiparametricEQEffect3KnobGroupState {
  public:
    SemiparametricEQEffect3KnobGroupState();
    inline ~SemiparametricEQEffect3KnobGroupState() {};

    EngineFilterBiquad1Low m_lowFilter;
    EngineFilterBiquad1Peaking m_semiParametricFilter;
    EngineFilterBiquad1High m_highFilter;

    SampleBuffer m_intermediateBuffer;
    ControlLogPotmeterBehavior m_filterBehavior;

    double m_dCenterOld;
    double m_dGainOld;
    double m_dFilterOld;
};

class SemiparametricEQEffect3Knob : public PerChannelEffectProcessor<SemiparametricEQEffect3KnobGroupState> {
  public:
    SemiparametricEQEffect3Knob(EngineEffect* pEffect, const EffectManifest& manifest);
    inline ~SemiparametricEQEffect3Knob() {};

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        SemiparametricEQEffect3KnobGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pCenter;
    EngineEffectParameter* m_pGain;
    EngineEffectParameter* m_pFilter;

    unsigned int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(SemiparametricEQEffect3Knob);
};

#endif // SEMIPARAMETRICEQ3KNOB_H
