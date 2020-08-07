#pragma once

#include <random>

#include "effects/effectprocessor.h"
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
    WhiteNoiseGroupState(const mixxx::EngineParameters& bufferParameters)
            : EffectState(bufferParameters),
              previous_drywet(0.0),
              gen(rs()) {
    }
    ~WhiteNoiseGroupState() {
    }

    CSAMPLE_GAIN previous_drywet;
    std::random_device rs;
    std::mt19937 gen;
};

class WhiteNoiseEffect : public EffectProcessorImpl<WhiteNoiseGroupState> {
  public:
    WhiteNoiseEffect(EngineEffect* pEffect);
    virtual ~WhiteNoiseEffect();

    static QString getId();
    static EffectManifestPointer getManifest();

    // See effectprocessor.h
    void processChannel(const ChannelHandle& handle,
            WhiteNoiseGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures);

  private:
    EngineEffectParameter* m_pDryWetParameter;

    DISALLOW_COPY_AND_ASSIGN(WhiteNoiseEffect);
};
