#pragma once

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/samplebuffer.h"

class SemiparametricEQEffect4KnobGroupState : public EffectState {
  public:
    SemiparametricEQEffect4KnobGroupState(const mixxx::EngineParameters& bufferParameters);

    EngineFilterBiquad1High m_highFilter;
    EngineFilterBiquad1Peaking m_semiParametricFilter;
    EngineFilterBiquad1Low m_lowFilter;

    mixxx::SampleBuffer m_intermediateBuffer;

    double m_dHpfOld;
    double m_dCenterOld;
    double m_dGainOld;
    double m_dLpfOld;
};

class SemiparametricEQEffect4Knob
        : public EffectProcessorImpl<SemiparametricEQEffect4KnobGroupState> {
  public:
    SemiparametricEQEffect4Knob() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            SemiparametricEQEffect4KnobGroupState* channelState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pHPF;
    EngineEffectParameterPointer m_pCenter;
    EngineEffectParameterPointer m_pGain;
    EngineEffectParameterPointer m_pLPF;

    unsigned int m_oldSampleRate;

    DISALLOW_COPY_AND_ASSIGN(SemiparametricEQEffect4Knob);
};
