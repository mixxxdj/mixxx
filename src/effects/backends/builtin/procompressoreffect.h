#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "engine/engine.h"
#include "util/class.h"
#include "util/samplebuffer.h"

// Professional DJ Compressor
// Smooth compression for DJ mixing, based on Calf DSP compressor algorithms

class CompressorGroupState : public EffectState {
  public:
    CompressorGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters) {
        audioParametersChanged(engineParameters);
        clear();
    }
    ~CompressorGroupState() override = default;

    void audioParametersChanged(const mixxx::EngineParameters& engineParameters) {
        m_sampleRate = engineParameters.sampleRate();
    };

    void clear() {
        prev_threshold = 0.0f;
        prev_ratio = 1.0f;
        prev_attack = 0.0f;
        prev_release = 0.0f;
        prev_makeup = 0.0f;
        prev_gain = 1.0f;
        envelope = 0.0f;
        prev_env[0] = 0.0f;
        prev_env[1] = 0.0f;
    };

    float m_sampleRate;
    float prev_threshold;
    float prev_ratio;
    float prev_attack;
    float prev_release;
    float prev_makeup;
    float prev_gain;
    float envelope;
    float prev_env[2];
};

class ProCompressorEffect : public EffectProcessorImpl<CompressorGroupState> {
  public:
    ProCompressorEffect() = default;
    ~ProCompressorEffect() override = default;

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
    QString debugString() const {
        return getId();
    }

    EngineEffectParameterPointer m_pThresholdParameter;
    EngineEffectParameterPointer m_pRatioParameter;
    EngineEffectParameterPointer m_pAttackParameter;
    EngineEffectParameterPointer m_pReleaseParameter;
    EngineEffectParameterPointer m_pMakeupParameter;
    EngineEffectParameterPointer m_pDryWetParameter;

    DISALLOW_COPY_AND_ASSIGN(ProCompressorEffect);
};
