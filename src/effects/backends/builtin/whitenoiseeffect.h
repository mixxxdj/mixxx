#pragma once

#include <random>

#include "effects/backends/effectprocessor.h"
#include "engine/filters/enginefilterbiquad1.h"
#include "util/class.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class WhiteNoiseGroupState final : public EffectState {
  public:
    WhiteNoiseGroupState(const mixxx::EngineParameters& bufferParameters)
            : EffectState(bufferParameters),
              previous_gain(0.0f),
              previous_q(0.707106781),
              gen(rs()),
              m_highpass(bufferParameters.sampleRate(), 20.0, 0.707, false),
              m_lowpass(bufferParameters.sampleRate(), 20000.0, 0.707, false),
              m_noiseBuffer(bufferParameters.samplesPerBuffer()),
              m_filteredBuffer(bufferParameters.samplesPerBuffer()) {
    }

    CSAMPLE_GAIN previous_gain;
    double previous_q;
    std::random_device rs;
    std::mt19937 gen;

    EngineFilterBiquad1High m_highpass;
    EngineFilterBiquad1Low m_lowpass;

    mixxx::SampleBuffer m_noiseBuffer;
    mixxx::SampleBuffer m_filteredBuffer;
};

class WhiteNoiseEffect : public EffectProcessorImpl<WhiteNoiseGroupState> {
  public:
    WhiteNoiseEffect() = default;
    ~WhiteNoiseEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(
            WhiteNoiseGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

  private:
    EngineEffectParameterPointer m_pDryWetParameter;
    EngineEffectParameterPointer m_pQParameter;
    EngineEffectParameterPointer m_pGainParameter;

    DISALLOW_COPY_AND_ASSIGN(WhiteNoiseEffect);
};
