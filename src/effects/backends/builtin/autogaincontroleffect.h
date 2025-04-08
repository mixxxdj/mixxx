#pragma once

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class AutoGainControlGroupState : public EffectState {
  public:
    AutoGainControlGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        clear(engineParameters);
    }

    void clear(const mixxx::EngineParameters& engineParameters);

    void calculateCoeffsIfChanged(
            const mixxx::EngineParameters& engineParameters,
            double attackParamMs,
            double releaseParamMs);

    double state;
    double attackCoeff;
    double releaseCoeff;

    double previousAttackParamMs;
    double previousReleaseParamMs;
    mixxx::audio::SampleRate previousSampleRate;
};

class AutoGainControlEffect : public EffectProcessorImpl<AutoGainControlGroupState> {
  public:
    AutoGainControlEffect() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            AutoGainControlGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pThreshold;
    EngineEffectParameterPointer m_pTarget;
    EngineEffectParameterPointer m_pGain;
    EngineEffectParameterPointer m_pKnee;
    EngineEffectParameterPointer m_pAttack;
    EngineEffectParameterPointer m_pRelease;

    DISALLOW_COPY_AND_ASSIGN(AutoGainControlEffect);

    void applyAutoGainControl(AutoGainControlGroupState* pState,
            const mixxx::EngineParameters& engineParameters,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput);
};
