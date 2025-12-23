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
    CompressorGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        clear(engineParameters);
    }

    void clear(const mixxx::EngineParameters& engineParameters);

    void calculateCoeffsIfChanged(
            const mixxx::EngineParameters& engineParameters,
            double attackParamMs,
            double releaseParamMs);

    double stateDB;
    double attackCoeff;
    double releaseCoeff;
    CSAMPLE_GAIN makeUpGainState;
    CSAMPLE_GAIN makeUpCoeff;

    double previousAttackParamMs;
    double previousReleaseParamMs;
    SINT previousFramesPerBuffer;
    mixxx::audio::SampleRate previousSampleRate;
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

    CSAMPLE* applyCompression(CompressorGroupState* pState,
            const mixxx::EngineParameters& engineParameters,
            const CSAMPLE* pInput,
            CSAMPLE* pGainBuffer);

    void applyAutoMakeUp(CompressorGroupState* pState,
            SINT numSamples,
            CSAMPLE* pGainBuffer);
};
