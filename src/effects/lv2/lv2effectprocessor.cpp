#include "effects/lv2/lv2effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "controlobject.h"

#define MAX_PARAMS 100
#define MAX_BUFFER_LEN 160000

LV2EffectProcessor::LV2EffectProcessor(EngineEffect* pEngineEffect,
                                       const EffectManifest& manifest,
                                       const LilvPlugin* plugin,
                                       QList<int> audioPortIndices,
                                       QList<int> controlPortIndices) {
    m_sampleRate = ControlObject::get(ConfigKey("[Master]", "samplerate"));
    inputL = new float[MAX_BUFFER_LEN];
    inputR = new float[MAX_BUFFER_LEN];
    outputL = new float[MAX_BUFFER_LEN];
    outputR = new float[MAX_BUFFER_LEN];
    params = new float[MAX_PARAMS];

    handle = lilv_plugin_instantiate(plugin, m_sampleRate, NULL);
    const QList<EffectManifestParameter> effectManifestParameterList =
            manifest.parameters();

    // Initialize EngineEffectParameters
    foreach (EffectManifestParameter param, effectManifestParameterList) {
        m_parameters.append(pEngineEffect->getParameterById(param.id()));
    }

    for (int i = 0; i < m_parameters.size(); i++) {
        params[i] = m_parameters[i]->value();
        lilv_instance_connect_port(handle, controlPortIndices[i], &params[i]);
    }

    // We assume the audio ports are in the following order:
    // input_left, input_right, output_left, output_right
    lilv_instance_connect_port(handle, audioPortIndices[0], inputL);
    lilv_instance_connect_port(handle, audioPortIndices[1], inputR);
    lilv_instance_connect_port(handle, audioPortIndices[2], outputL);
    lilv_instance_connect_port(handle, audioPortIndices[3], outputR);

    lilv_instance_activate(handle);
}

LV2EffectProcessor::~LV2EffectProcessor() {
    lilv_instance_deactivate(handle);
    lilv_instance_free(handle);
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
        params[i] = m_parameters[i]->value();
    }

    int j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        inputL[j] = pInput[i];
        inputR[j] = pInput[i + 1];
        j++;
    }

    lilv_instance_run(handle, numSamples / 2);

    j = 0;
    for (unsigned int i = 0; i < numSamples; i += 2) {
        pOutput[i] = outputL[j];
        pOutput[i + 1] = outputR[j];
        j++;
    }
}
