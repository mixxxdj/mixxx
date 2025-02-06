#include "effects/backends/lv2/lv2effectprocessor.h"

#include "engine/effects/engineeffectparameter.h"
#include "util/defs.h"

LV2EffectProcessor::LV2EffectProcessor(LV2EffectManifestPointer pManifest)
        : m_pManifest(pManifest),
          m_LV2parameters(nullptr),
          m_pPlugin(pManifest->getPlugin()),
          m_audioPortIndices(pManifest->getAudioPortIndices()),
          m_controlPortIndices(pManifest->getControlPortIndices()) {
    m_inputL = new float[kMaxEngineSamples];
    m_inputR = new float[kMaxEngineSamples];
    m_outputL = new float[kMaxEngineSamples];
    m_outputR = new float[kMaxEngineSamples];
}

void LV2EffectProcessor::loadEngineEffectParameters(
        const QMap<QString, EngineEffectParameterPointer>& parameters) {
    m_LV2parameters = new float[parameters.size()];

    // EngineEffect passes the EngineEffectParameters indexed by ID string, which
    // is used directly by built-in EffectProcessorImpl subclasseses to access
    // specific named parameters. However, LV2EffectProcessor::process iterates
    // over the EngineEffectParameters to copy their values to the LV2 control
    // ports. To avoid slow string comparisons in the audio engine thread in
    // LV2EffectProcessor::process, rearrange the QMap of EngineEffectParameters by
    // ID string to an ordered QList.
    for (const auto& pManifestParameter : m_pManifest->parameters()) {
        m_engineEffectParameters.append(parameters.value(pManifestParameter->id()));
    }
}

LV2EffectProcessor::~LV2EffectProcessor() {
    delete[] m_inputL;
    delete[] m_inputR;
    delete[] m_outputL;
    delete[] m_outputR;
    delete[] m_LV2parameters;
}

void LV2EffectProcessor::processChannel(
        LV2EffectGroupState* channelState,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
        const mixxx::EngineParameters& engineParameters,
        const EffectEnableState enableState,
        const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);

    for (int i = 0; i < m_engineEffectParameters.size(); i++) {
        m_LV2parameters[i] = static_cast<float>(m_engineEffectParameters[i]->value());
    }

    SINT framesPerBuffer = engineParameters.framesPerBuffer();
    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < framesPerBuffer; ++i) {
        m_inputL[i] = pInput[i * 2];
        m_inputR[i] = pInput[i * 2 + 1];
    }

    LilvInstance* instance = channelState->lilvInstance(m_pPlugin, engineParameters);

    if (enableState == EffectEnableState::Enabling) {
        lilv_instance_activate(instance);
    }

    lilv_instance_run(instance, framesPerBuffer);

    // note: LOOP VECTORIZED.
    for (SINT i = 0; i < framesPerBuffer; ++i) {
        pOutput[i * 2] = m_outputL[i];
        pOutput[i * 2 + 1] = m_outputR[i];
    }

    if (enableState == EffectEnableState::Disabling) {
        lilv_instance_deactivate(instance);
    }
}

LV2EffectGroupState* LV2EffectProcessor::createSpecificState(
        const mixxx::EngineParameters& engineParameters) {
    LV2EffectGroupState* pState = new LV2EffectGroupState(engineParameters);
    LilvInstance* pInstance = pState->lilvInstance(m_pPlugin, engineParameters);
    VERIFY_OR_DEBUG_ASSERT(pInstance) {
        return pState;
    }

    if (kEffectDebugOutput) {
        qDebug() << this << "LV2EffectProcessor creating LV2EffectGroupState" << pState;
    }

    if (pInstance) {
        for (int i = 0; i < m_engineEffectParameters.size(); i++) {
            m_LV2parameters[i] = static_cast<float>(m_engineEffectParameters[i]->value());
            lilv_instance_connect_port(pInstance,
                    m_controlPortIndices[i],
                    &m_LV2parameters[i]);
        }

        // We assume the audio ports are in the following order:
        // input_left, input_right, output_left, output_right
        lilv_instance_connect_port(pInstance, m_audioPortIndices[0], m_inputL);
        lilv_instance_connect_port(pInstance, m_audioPortIndices[1], m_inputR);
        lilv_instance_connect_port(pInstance, m_audioPortIndices[2], m_outputL);
        lilv_instance_connect_port(pInstance, m_audioPortIndices[3], m_outputR);
    }
    return pState;
};
