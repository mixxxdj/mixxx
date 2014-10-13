#include "effects/lv2/lv2effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "controlobject.h"

#define MAX_BUFFER_LEN 160000

LV2EffectProcessor::LV2EffectProcessor(EngineEffect* pEngineEffect,
                                       const EffectManifest& manifest,
                                       const LilvPlugin* plugin,
                                       QList<int> audioPortIndices,
                                       QList<int> controlPortIndices) {
    m_sampleRate = ControlObject::get(ConfigKey("[Master]", "samplerate"));
    m_inputL = new float[MAX_BUFFER_LEN];
    m_inputR = new float[MAX_BUFFER_LEN];
    m_outputL = new float[MAX_BUFFER_LEN];
    m_outputR = new float[MAX_BUFFER_LEN];
    m_params = new float[manifest.parameters().size()];

    m_handle = lilv_plugin_instantiate(plugin, m_sampleRate, NULL);
    const QList<EffectManifestParameter> effectManifestParameterList =
            manifest.parameters();

    // Initialize EngineEffectParameters
    foreach (EffectManifestParameter param, effectManifestParameterList) {
        m_parameters.append(pEngineEffect->getParameterById(param.id()));
    }

    for (int i = 0; i < m_parameters.size(); i++) {
        m_params[i] = m_parameters[i]->value();
        lilv_instance_connect_port(m_handle, controlPortIndices[i], &m_params[i]);
    }

    // We assume the audio ports are in the following order:
    // input_left, input_right, output_left, output_right
    lilv_instance_connect_port(m_handle, audioPortIndices[0], m_inputL);
    lilv_instance_connect_port(m_handle, audioPortIndices[1], m_inputR);
    lilv_instance_connect_port(m_handle, audioPortIndices[2], m_outputL);
    lilv_instance_connect_port(m_handle, audioPortIndices[3], m_outputR);

    lilv_instance_activate(m_handle);
}

LV2EffectProcessor::~LV2EffectProcessor() {
    lilv_instance_deactivate(m_handle);
    lilv_instance_free(m_handle);
    delete[] m_inputL;
    delete[] m_inputR;
    delete[] m_outputL;
    delete[] m_outputR;
    delete[] m_params;
}

void LV2EffectProcessor::initialize(const QSet<QString>& registeredGroups) {
    Q_UNUSED(registeredGroups);

}

void LV2EffectProcessor::process(const QString& group,
                         const CSAMPLE* pInput, CSAMPLE* pOutput,
                         const unsigned int numSamples,
                         const unsigned int sampleRate,
                         const GroupFeatureState& groupFeatures) {
    Q_UNUSED(groupFeatures);
    Q_UNUSED(sampleRate);
    Q_UNUSED(group);
    for (int i = 0; i < m_parameters.size(); i++) {
        m_params[i] = m_parameters[i]->value();
    }

    int j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        m_inputL[j] = pInput[i];
        m_inputR[j] = pInput[i + 1];
        j++;
    }

    lilv_instance_run(m_handle, numSamples / 2);

    j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        pOutput[i] = m_outputL[j];
        pOutput[i + 1] = m_outputR[j];
        j++;
    }
}
