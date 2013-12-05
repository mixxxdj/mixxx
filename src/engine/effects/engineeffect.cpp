#include "engine/effects/engineeffect.h"

EngineEffect::EngineEffect(const EffectManifest& manifest,
                           EffectInstantiatorPointer pInstantiator)
        : m_manifest(manifest),
          m_parameters(manifest.parameters().size()) {
    const QList<EffectManifestParameter>& parameters = m_manifest.parameters();
    for (int i = 0; i < parameters.size(); ++i) {
        const EffectManifestParameter& parameter = parameters.at(i);
        EngineEffectParameter* pParameter =
                new EngineEffectParameter(parameter);
        m_parameters[i] = pParameter;
        m_parametersById[parameter.id()] = pParameter;
    }

    // Creating the processor must come last.
    m_pProcessor = pInstantiator->instantiate(this, manifest);
}

EngineEffect::~EngineEffect() {
    qDebug() << debugString() << "destroyed";
    delete m_pProcessor;
    m_parametersById.clear();
    for (int i = 0; i < m_parameters.size(); ++i) {
        EngineEffectParameter* pParameter = m_parameters.at(i);
        m_parameters[i] = NULL;
        delete pParameter;
    }
}

bool EngineEffect::processEffectsRequest(const EffectsRequest& message,
                                         EffectsResponsePipe* pResponsePipe) {
    EngineEffectParameter* pParameter = NULL;
    EffectsResponse response(message);

    switch (message.type) {
        case EffectsRequest::SET_EFFECT_PARAMETER:
            qDebug() << debugString() << "SET_EFFECT_PARAMETER"
                     << "parameter" << message.SetEffectParameter.iParameter
                     << "minimum" << message.minimum
                     << "maximum" << message.maximum
                     << "default_value" << message.default_value
                     << "value" << message.value;
            pParameter = m_parameters.value(
                message.SetEffectParameter.iParameter, NULL);
            if (pParameter) {
                pParameter->setMinimum(message.minimum);
                pParameter->setMaximum(message.maximum);
                pParameter->setDefaultValue(message.default_value);
                pParameter->setValue(message.value);
                response.success = true;
            } else {
                response.success = false;
                response.status = EffectsResponse::NO_SUCH_PARAMETER;
            }
            pResponsePipe->writeMessages(&response, 1);
            return true;
        default:
            break;
    }
    return false;
}

void EngineEffect::process(const QString& group,
                           const CSAMPLE* pInput, CSAMPLE* pOutput,
                           const unsigned int numSamples) {
    m_pProcessor->process(group, pInput, pOutput, numSamples);
}
