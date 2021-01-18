#pragma once

#include "effects/effectprocessor.h"
#include "effects/effectmanifest.h"
#include "engine/effects/engineeffectparameter.h"
#include <lilv-0/lilv/lilv.h>
#include "effects/defs.h"
#include "engine/engine.h"

class LV2EffectGroupState : public EffectState {
  public:
    LV2EffectGroupState(const mixxx::EngineParameters& bufferParameters, const LilvPlugin* pPlugin)
            : EffectState(bufferParameters) {
        m_pInstance = lilv_plugin_instantiate(pPlugin, bufferParameters.sampleRate(), nullptr);
    }
    ~LV2EffectGroupState() {
        lilv_instance_deactivate(m_pInstance);
        lilv_instance_free(m_pInstance);
    }

    LilvInstance* lilvIinstance() {
        return m_pInstance;
    }
  private:
    LilvInstance* m_pInstance;
};

class LV2EffectProcessor : public EffectProcessor {
  public:
    LV2EffectProcessor(EngineEffect* pEngineEffect,
            EffectManifestPointer pManifest,
            const LilvPlugin* plugin,
            const QList<int>& audioPortIndices,
            const QList<int>& controlPortIndices);
    ~LV2EffectProcessor();

    void initialize(
            const QSet<ChannelHandleAndGroup>& activeInputChannels,
            EffectsManager* pEffectsManager,
            const mixxx::EngineParameters& bufferParameters) override;
    EffectState* createState(const mixxx::EngineParameters& bufferParameters) final;
    bool loadStatesForInputChannel(const ChannelHandle* inputChannel,
          const EffectStatesMap* pStatesMap) override;
    // Called from main thread for garbage collection after the last audio thread
    // callback executes process() with EffectEnableState::Disabling
    void deleteStatesForInputChannel(const ChannelHandle* inputChannel) override;

    void process(const ChannelHandle& inputHandle,
            const ChannelHandle& outputHandle,
            const CSAMPLE* pInput, CSAMPLE* pOutput,
            const mixxx::EngineParameters& bufferParameters,
            const EffectEnableState enableState,
            const GroupFeatureState& groupFeatures) override;
  private:
    LV2EffectGroupState* createGroupState(const mixxx::EngineParameters& bufferParameters);

    QList<EngineEffectParameter*> m_parameters;
    float* m_inputL;
    float* m_inputR;
    float* m_outputL;
    float* m_outputR;
    float* m_params;
    const LilvPlugin* m_pPlugin;
    const QList<int> m_audioPortIndices;
    const QList<int> m_controlPortIndices;

    EffectsManager* m_pEffectsManager;
    ChannelHandleMap<ChannelHandleMap<LV2EffectGroupState*>> m_channelStateMatrix;
};
