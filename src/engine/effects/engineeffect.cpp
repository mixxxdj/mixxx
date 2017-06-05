#include "engine/effects/engineeffect.h"

#include "util/sample.h"

EngineEffect::EngineEffect(const EffectManifest& manifest,
                           const QSet<ChannelHandleAndGroup>& registeredChannels,
                           EffectInstantiatorPointer pInstantiator)
        : m_manifest(manifest),
          m_bEffectSlotEnabled(false),
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
    m_pProcessor->initialize(registeredChannels);
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

            m_bEffectSlotEnabled = message.SetEffectParameters.enabled;

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

bool EngineEffect::process(const ChannelHandle& handle,
                           const CSAMPLE* pInput, CSAMPLE* pOutput,
                           const unsigned int numSamples,
                           const unsigned int sampleRate,
                           const EffectProcessor::EnableState chainEnableState,
                           const GroupFeatureState& groupFeatures) {
    if (!m_channelEnableState.contains(handle)) {
        m_channelEnableState[handle] = EffectProcessor::DISABLED;
    }

    EffectProcessor::EnableState effectiveEnableState = m_channelEnableState.value(handle);

    // Does the enabled state need to change from the last callback?
    if (effectiveEnableState == EffectProcessor::DISABLED) {
        if (m_bEffectSlotEnabled
               && (chainEnableState == EffectProcessor::ENABLING
                   || chainEnableState == EffectProcessor::ENABLED)) {
            effectiveEnableState = EffectProcessor::ENABLING;
            m_channelEnableState[handle] = EffectProcessor::ENABLED;
        } else {
            // Nothing to do, skip processing.
            return false;
        }
    } else if (effectiveEnableState == EffectProcessor::ENABLED
                  && (!m_bEffectSlotEnabled
                      || chainEnableState == EffectProcessor::DISABLING)) {
        effectiveEnableState = EffectProcessor::DISABLING;
        m_channelEnableState[handle] = EffectProcessor::DISABLING;
    }

    // Do processing.
    if (effectiveEnableState == EffectProcessor::ENABLING) {
        m_pProcessor->process(handle, pInput, pOutput, numSamples, sampleRate,
            EffectProcessor::ENABLING, groupFeatures);
        if (!m_effectRampsFromDry) {
            DEBUG_ASSERT(pInput != pOutput); // Fade to dry only works if pInput is not touched by pOutput
            // Fade in (fade to wet signal)
            SampleUtil::copy2WithRampingGain(pOutput,
                    pInput, 1.0, 0.0,
                    pOutput, 0.0, 1.0,
                    numSamples);
        }
    } else if (effectiveEnableState == EffectProcessor::DISABLING) {
        CSAMPLE* silence = SampleUtil::alloc(numSamples);
        SampleUtil::fill(silence, CSAMPLE_ZERO, numSamples);

        m_pProcessor->process(handle, silence, pOutput, numSamples, sampleRate,
            EffectProcessor::DISABLING, groupFeatures);

        // When output level becomes inaudible, stop processing. This allows
        // temporal effects like Echo to continue outputting their tail.
        // NOTE: The 1 threshold is totally arbitrary.
        CSAMPLE fVolSumL, fVolSumR;
        SampleUtil::sumAbsPerChannel(&fVolSumL, &fVolSumR, pOutput, numSamples);
        if (fVolSumL < 1 && fVolSumR < 1) {
            m_channelEnableState[handle] = EffectProcessor::DISABLED;
            if (!m_effectRampsFromDry) {
                // Fade out (fade to dry signal)
                // Fading to dry only works if pInput is not touched by pOutput
                DEBUG_ASSERT(pInput != pOutput);
                SampleUtil::copy2WithRampingGain(pOutput,
                    pInput, 0.0, 1.0,
                    pOutput, 1.0, 0.0,
                    numSamples);
            } else {
                // Just copy
                SampleUtil::addWithGain(pOutput, pInput, 1.0, numSamples);
            }
        } else {
            SampleUtil::addWithGain(pOutput, pInput, 1.0, numSamples);
        }
    } else if (effectiveEnableState == EffectProcessor::ENABLED) {
        m_pProcessor->process(handle, pInput, pOutput, numSamples, sampleRate,
            EffectProcessor::ENABLED, groupFeatures);
    }
    return true;
}
