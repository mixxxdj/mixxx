#pragma once

#include <QMap>

#include "effects/backends/effectprocessor.h"
#include "util/class.h"
#include "util/types.h"

class MetronomeGroupState final : public EffectState {
  public:
    MetronomeGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              m_framesSinceClickStart(0) {
    }
    ~MetronomeGroupState() override = default;

    SINT m_framesSinceClickStart;
};

class MetronomeEffect : public EffectProcessorImpl<MetronomeGroupState> {
  public:
    MetronomeEffect() = default;
    ~MetronomeEffect() override = default;

    static QString getId();
    static EffectManifestPointer getManifest();

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            MetronomeGroupState* pState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    EngineEffectParameterPointer m_pBpmParameter;
    EngineEffectParameterPointer m_pSyncParameter;

    DISALLOW_COPY_AND_ASSIGN(MetronomeEffect);
};
