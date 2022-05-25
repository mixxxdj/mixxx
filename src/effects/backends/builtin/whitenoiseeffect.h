#pragma once

#include <stdint.h>

#include <random>

#include "effects/backends/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/filters/enginefilterpansingle.h"
#include "util/class.h"
#include "util/defs.h"
#include "util/sample.h"
#include "util/samplebuffer.h"
#include "util/types.h"

class WhiteNoiseGroupState final : public EffectState {
  public:
    WhiteNoiseGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              previous_drywet(0.0),
              random_state(1) {
    }
    ~WhiteNoiseGroupState() {
    }

    CSAMPLE_GAIN previous_drywet;
    uint32_t random_state;
};

class WhiteNoiseEffect : public EffectProcessorImpl<WhiteNoiseGroupState> {
  public:
    WhiteNoiseEffect() = default;

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
