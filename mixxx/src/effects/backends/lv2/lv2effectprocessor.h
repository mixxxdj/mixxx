#pragma once

#include <lilv/lilv.h>

#include "effects/backends/effectprocessor.h"
#include "effects/backends/lv2/lv2manifest.h"
#include "effects/defs.h"
#include "engine/engine.h"

// Refer to EffectProcessor for documentation
class LV2EffectGroupState final : public EffectState {
  public:
    LV2EffectGroupState(const mixxx::EngineParameters& engineParameters)
            : EffectState(engineParameters),
              m_pInstance(nullptr) {
    }

    ~LV2EffectGroupState() override {
        if (m_pInstance) {
            lilv_instance_deactivate(m_pInstance);
            lilv_instance_free(m_pInstance);
        }
    }

    LilvInstance* lilvInstance(const LilvPlugin* pPlugin,
            const mixxx::EngineParameters& engineParameters) {
        if (!m_pInstance) {
            m_pInstance = lilv_plugin_instantiate(
                    pPlugin, engineParameters.sampleRate(), nullptr);
        }
        return m_pInstance;
    }

  private:
    LilvInstance* m_pInstance;
};

class LV2EffectProcessor final : public EffectProcessorImpl<LV2EffectGroupState> {
  public:
    LV2EffectProcessor(LV2EffectManifestPointer pManifest);
    ~LV2EffectProcessor() override;

    void loadEngineEffectParameters(
            const QMap<QString, EngineEffectParameterPointer>& parameters) override;

    void processChannel(
            LV2EffectGroupState* channelState,
            const CSAMPLE* pInput,
            CSAMPLE* pOutput,
            const mixxx::EngineParameters& engineParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;

  private:
    LV2EffectGroupState* createSpecificState(
            const mixxx::EngineParameters& engineParameters) override;

    LV2EffectManifestPointer m_pManifest;
    QList<EngineEffectParameterPointer> m_engineEffectParameters;
    float* m_inputL;
    float* m_inputR;
    float* m_outputL;
    float* m_outputR;
    float* m_LV2parameters;
    const LilvPlugin* m_pPlugin;
    const QList<int> m_audioPortIndices;
    const QList<int> m_controlPortIndices;
};
