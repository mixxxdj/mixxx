#include "effects/lv2/lv2effectprocessor.h"
#include "engine/effects/engineeffect.h"

LV2EffectProcessor::LV2EffectProcessor(EngineEffect* pEngineEffect,
                                       const EffectManifest& manifest) {
    const QList<EffectManifestParameter> effectManifestParameterList = manifest.parameters();
    // Initialize EngineEffectParameters
    foreach (EffectManifestParameter param, effectManifestParameterList) {
        m_parameters.append(pEngineEffect->getParameterById(param.id()));
    }
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
    foreach (EngineEffectParameter* param, m_parameters) {
        qDebug() <<"EngineEffectParameter: " << param->value();
    }
    for (unsigned int i = 0; i < numSamples; i++) {
        pOutput[i] = 0;
    }
}
