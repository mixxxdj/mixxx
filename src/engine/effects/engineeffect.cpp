#include "engine/effects/engineeffect.h"
#include "sampleutil.h"


EngineEffect::EngineEffect(const EffectManifest& manifest,
                           const QSet<QString>& registeredGroups,
                           EffectInstantiatorPointer pInstantiator)
        : m_manifest(manifest),
          m_enableState(EffectProcessor::ENABLING),
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
    m_pProcessor->initialize(registeredGroups);
    m_effectRampsFromDry = manifest.effectRampsFromDry();
}

EngineEffect::~EngineEffect() {
    if (kEffectDebugOutput) {
        qDebug() << debugString() << "destroyed";
    }
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
        case EffectsRequest::SET_EFFECT_PARAMETERS:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "SET_EFFECT_PARAMETERS"
                         << "enabled" << message.SetEffectParameters.enabled;
            }

            if (m_enableState != EffectProcessor::DISABLED && !message.SetEffectParameters.enabled) {
                m_enableState = EffectProcessor::DISABLING;
            } else if (m_enableState == EffectProcessor::DISABLED && message.SetEffectParameters.enabled) {
                m_enableState = EffectProcessor::ENABLING;
            }

            response.success = true;
            pResponsePipe->writeMessages(&response, 1);
            return true;
            break;
        case EffectsRequest::SET_PARAMETER_PARAMETERS:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "SET_PARAMETER_PARAMETERS"
                         << "parameter" << message.SetParameterParameters.iParameter
                         << "minimum" << message.minimum
                         << "maximum" << message.maximum
                         << "default_value" << message.default_value
                         << "value" << message.value;
            }
            pParameter = m_parameters.value(
                message.SetParameterParameters.iParameter, NULL);
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
                           const unsigned int numSamples,
                           const unsigned int sampleRate,
                           const EffectProcessor::EnableState enableState,
                           const GroupFeatureState& groupFeatures) {
    EffectProcessor::EnableState effectiveEnableState = m_enableState;
    if (enableState == EffectProcessor::DISABLING) {
        effectiveEnableState = EffectProcessor::DISABLING;
    } else if (enableState == EffectProcessor::ENABLING) {
        effectiveEnableState = EffectProcessor::ENABLING;
    }

    m_pProcessor->process(group, pInput, pOutput, numSamples, sampleRate,
            effectiveEnableState, groupFeatures);
    if (!m_effectRampsFromDry) {
        // the effect does not fade, so we care for it
        if (effectiveEnableState == EffectProcessor::DISABLING) {
            // Fade out (fade to dry signal)
            SampleUtil::copy2WithRampingGain(pOutput,
                    pInput, 0.0, 1.0,
                    pOutput, 1.0, 0.0,
                    numSamples);
        } else if (effectiveEnableState == EffectProcessor::ENABLING) {
            // Fade in (fade to wet signal)
            SampleUtil::copy2WithRampingGain(pOutput,
                    pInput, 1.0, 0.0,
                    pOutput, 0.0, 1.0,
                    numSamples);
        }
    }

    if (m_enableState == EffectProcessor::DISABLING) {
        m_enableState = EffectProcessor::DISABLED;
    } else if (m_enableState == EffectProcessor::ENABLING) {
        m_enableState = EffectProcessor::ENABLED;
    }
}
