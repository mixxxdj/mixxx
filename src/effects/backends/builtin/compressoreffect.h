#pragma once

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/types.h"

namespace {
constexpr double makeUpAttackCoeff = 0.03;
const double makeUpCoeff = 1 / pow(10, 0.15);
} // anonymous namespace

class CompressorGroupState : public EffectState {
  public:
    CompressorGroupState(const mixxx::EngineParameters& engineParameters);

    double samplerate;
    CSAMPLE previousStateDB;
    CSAMPLE previousMakeUpGain;
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

    enum AutoMakeUp {
        AutoMakeUpOff = 0,
        AutoMakeUpOn = 1,
    };

    enum Clipping {
        ClippingOff = 0,
        ClippingOn = 1,
    };

    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pClipping;
    EngineEffectParameterPointer m_pAutoMakeUp;
    EngineEffectParameterPointer m_pThreshold;
    EngineEffectParameterPointer m_pRatio;
    EngineEffectParameterPointer m_pKnee;
    EngineEffectParameterPointer m_pAttack;
    EngineEffectParameterPointer m_pRelease;
    EngineEffectParameterPointer m_pGain;

    DISALLOW_COPY_AND_ASSIGN(CompressorEffect);

    void applyCompression(CompressorGroupState* pState, const SINT& numSamples, int channelCount, const CSAMPLE* pInput, CSAMPLE* pOutput);
};
