#pragma once

#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/samplebuffer.h"

class SemiparametricEQEffect3KnobGroupState : public EffectState {
  public:
    SemiparametricEQEffect3KnobGroupState(const mixxx::EngineParameters& bufferParameters);

    EngineFilterBiquad1High m_highFilter;
    EngineFilterBiquad1Peaking m_semiParametricFilter;
    EngineFilterBiquad1Low m_lowFilter;

    mixxx::SampleBuffer m_intermediateBuffer;
    ControlLogPotmeterBehavior m_filterBehavior;

    double m_dCenterOld;
    double m_dGainOld;
    double m_dFilterOld;
};

class SemiparametricEQEffect3Knob
        : public EffectProcessorImpl<SemiparametricEQEffect3KnobGroupState> {
  public:
    SemiparametricEQEffect3Knob(EngineEffect* pEffect);

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(const ChannelHandle& handle,
            SemiparametricEQEffect3KnobGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatureState) override;

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
