#ifndef SEMIPARAMETRICEQ4KNOB_H
#define SEMIPARAMETRICEQ4KNOB_H

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

struct SemiparametricEQEffect4KnobGroupState {
    SemiparametricEQEffect4KnobGroupState();
    inline ~SemiparametricEQEffect4KnobGroupState() {};

    EngineFilterBiquad1Low m_lowFilter;
    EngineFilterBiquad1Peaking m_semiParametricFilter;
    EngineFilterBiquad1High m_highFilter;

    SampleBuffer m_intermediateBuffer;

    double m_dLpfOld;
    double m_dCenterOld;
    double m_dGainOld;
    double m_dHpfOld;
};

class SemiparametricEQEffect4Knob : public PerChannelEffectProcessor<SemiparametricEQEffect4KnobGroupState> {
public:
    SemiparametricEQEffect4Knob(EngineEffect* pEffect, const EffectManifest& manifest);
    inline ~SemiparametricEQEffect4Knob() {};

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
                        SemiparametricEQEffect4KnobGroupState* pState,
                        const CSAMPLE* pInput, CSAMPLE *pOutput,
                        const unsigned int numSamples,
                        const unsigned int sampleRate,
                        const EffectProcessor::EnableState enableState,
                        const GroupFeatureState& groupFeatureState);

private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameter* m_pLPF;
    EngineEffectParameter* m_pCenter;
    EngineEffectParameter* m_pGain;
    EngineEffectParameter* m_pHPF;

    unsigned int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(SemiparametricEQEffect4Knob);
};

#endif // SEMIPARAMETRICEQ4KNOB_H
