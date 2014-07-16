#include "effects/lv2/lv2effectprocessor.h"
#include "engine/effects/engineeffect.h"

LV2EffectProcessor::LV2EffectProcessor(EngineEffect* pEngineEffect,
                                       const EffectManifest& manifest,
                                       const LilvPlugin* plugin) {
    inputL = new float[10000];
    inputR = new float[10000];
    outputL = new float[10000];
    outputR = new float[10000];
    params = new float[100];
    handle = lilv_plugin_instantiate(plugin, 44100, NULL);
    const QList<EffectManifestParameter> effectManifestParameterList = manifest.parameters();
    // Initialize EngineEffectParameters
    foreach (EffectManifestParameter param, effectManifestParameterList) {
        m_parameters.append(pEngineEffect->getParameterById(param.id()));
    }

    for (int i = 0; i < m_parameters.size(); i++) {
        params[i] = m_parameters[i]->value();
        lilv_instance_connect_port(handle, i + 4, &params[i]);
    }

    // Only for Calf Flanger, we are hard coding the indexes
    lilv_instance_connect_port(handle, 0, inputL);
    lilv_instance_connect_port(handle, 1, inputR);
    lilv_instance_connect_port(handle, 2, outputL);
    lilv_instance_connect_port(handle, 3, outputR);

    lilv_instance_activate(handle);
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
    for (int i = 0; i < m_parameters.size(); i++) {
        params[i] = m_parameters[i]->value();
    }

    int j = 0;
    for (int i = 0; i < numSamples; i += 2) {
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
