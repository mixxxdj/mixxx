#pragma once

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

class CompressorGroupState : public EffectState {
  public:
    CompressorGroupState(const mixxx::EngineParameters& engineParameters);

    double previousStateDB;
    double previousAttackParamMs;
    double previousAttackCoeff;
    double previousReleaseParamMs;
    double previousReleaseCoeff;
    CSAMPLE_GAIN previousMakeUpGain;
};

class CompressorEffect : public EffectProcessorImpl<CompressorGroupState> {
  public:
    CompressorEffect() = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            CompressorGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    enum class AutoMakeUp {
        AutoMakeUpOff = 0,
        AutoMakeUpOn = 1,
    };

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pAutoMakeUp;
    EngineEffectParameterPointer m_pThreshold;
    EngineEffectParameterPointer m_pRatio;
    EngineEffectParameterPointer m_pKnee;
    EngineEffectParameterPointer m_pAttack;
    EngineEffectParameterPointer m_pRelease;
    EngineEffectParameterPointer m_pLevel;

    DISALLOW_COPY_AND_ASSIGN(CompressorEffect);

    void applyCompression(CompressorGroupState* pState,
            const mixxx::EngineParameters& engineParameters,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput);

    void applyAutoMakeUp(CompressorGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const SINT& numSamples);
};
