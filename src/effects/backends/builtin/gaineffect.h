#pragma once

#include "effects/backends/effectprocessor.h"
#include "util/class.h"

struct GainGroupState : public EffectState {
  public:
    GainGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              previous_gain(1.0) {
    }
    ~GainGroupState() override = default;
    CSAMPLE_GAIN previous_gain;
};

class GainEffect : public EffectProcessorImpl<GainGroupState> {
  public:
    GainEffect() = default;
    ~GainEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void processChannel(
            GainGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

  private:
    EngineEffectParameterPointer m_pGainParameter;
    EngineEffectParameterPointer m_pClipParameter;

    DISALLOW_COPY_AND_ASSIGN(GainEffect);
};
