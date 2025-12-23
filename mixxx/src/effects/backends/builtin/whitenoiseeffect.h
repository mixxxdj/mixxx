#pragma once

#include <random>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

class WhiteNoiseGroupState final : public EffectState {
  public:
    WhiteNoiseGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              previous_drywet(0.0),
              gen(rs()) {
    }
    ~WhiteNoiseGroupState() override = default;

    CSAMPLE_GAIN previous_drywet;
    std::random_device rs;
    std::mt19937 gen;
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

    DISALLOW_COPY_AND_ASSIGN(WhiteNoiseEffect);
};
