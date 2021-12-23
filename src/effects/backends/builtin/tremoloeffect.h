#pragma once
#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class TremoloState : public EffectState {
  public:
    TremoloState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters){};
    double gain;
    unsigned int currentFrame;
    bool quantizeEnabled = false;
    bool tripletEnabled = false;
};

class TremoloEffect : public EffectProcessorImpl<TremoloState> {
  public:
    TremoloEffect() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            TremoloState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pDepthParameter;
    EngineEffectParameterPointer m_pRateParameter;
    EngineEffectParameterPointer m_pWidthParameter;
    EngineEffectParameterPointer m_pWaveformParameter;
    EngineEffectParameterPointer m_pPhaseParameter;
    EngineEffectParameterPointer m_pQuantizeParameter;
    EngineEffectParameterPointer m_pTripletParameter;

    DISALLOW_COPY_AND_ASSIGN(TremoloEffect);
};
