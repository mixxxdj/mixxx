#include "engine/effects/engineeffect.h"

#include "util/sample.h"

EngineEffect::EngineEffect(const EffectManifest& manifest,
                           const QSet<ChannelHandleAndGroup>& registeredInputChannels,
                           const QSet<ChannelHandleAndGroup>& registeredOutputChannels,
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

    for (const ChannelHandleAndGroup& inputChannel : registeredInputChannels) {
        ChannelHandleMap<EffectProcessor::EnableState> outputChannelMap;
        for (const ChannelHandleAndGroup& outputChannel : registeredOutputChannels) {
            outputChannelMap.insert(outputChannel.handle(), EffectProcessor::DISABLED);
        }
        m_effectEnableStateForChannelMatrix.insert(inputChannel.handle(), outputChannelMap);
    }

    // Creating the processor must come last.
    m_pProcessor = pInstantiator->instantiate(this, manifest);
    m_pProcessor->initialize(registeredInputChannels, registeredOutputChannels);
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

            for (auto& outputMap : m_effectEnableStateForChannelMatrix) {
                for (auto& enableState : outputMap) {
                    if (enableState != EffectProcessor::DISABLED
                        && !message.SetEffectParameters.enabled) {
                        enableState = EffectProcessor::DISABLING;
                    // If an input is not routed to the chain, and the effect gets
                    // a message to disable, then the effect gets the message to enable,
                    // process() will not have executed, so the enableState will still be
                    // DISABLING instead of DISABLED.
                    } else if ((enableState == EffectProcessor::DISABLED ||
                               enableState == EffectProcessor::DISABLING)
                               && message.SetEffectParameters.enabled) {
                        enableState = EffectProcessor::ENABLING;
                    }
                }
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

bool EngineEffect::process(const ChannelHandle& inputHandle,
                           const ChannelHandle& outputHandle,
                           const CSAMPLE* pInput, CSAMPLE* pOutput,
                           const unsigned int numSamples,
                           const unsigned int sampleRate,
                           const EffectProcessor::EnableState chainEnableState,
                           const GroupFeatureState& groupFeatures) {
    // Compute the effective enable state from the combination of the effect's state
    // for the channel and the state passed from the EngineEffectChain.

    // When the chain's input routing switch or chain enable switches are changed,
    // the chain sends an intermediate enabling/disabling signal. The chain also sends
    // intermediate enabling/disabling signals when its dry/wet knob is turned down to
    // fully dry then turned back up to let some wet signal through.

    // Analagously, when the Effect is switched on/off, it sends this EngineEffect an
    // intermediate enabling/disabling signal.

    // The effective enable state is then passed down to the EffectProcessor, which is
    // responsible for taking appropriate action when it gets an intermediate
    // enabling/disabling signal. For example, the Echo effect clears its
    // internal buffer for the channel when it gets the intermediate disabling signal.

    EffectProcessor::EnableState effectiveEffectEnableState =
        m_effectEnableStateForChannelMatrix[inputHandle][outputHandle];

    // If the EngineEffect is fully disabled, do not let
    // intermediate enabling/disabing signals from the chain override
    // the EngineEffect's state.
    if (effectiveEffectEnableState != EffectProcessor::DISABLED) {
        if (chainEnableState == EffectProcessor::DISABLED) {
            // If the chain is fully disabled, skip calling the EffectProcessor.
            effectiveEffectEnableState = EffectProcessor::DISABLED;
        } else if (chainEnableState == EffectProcessor::DISABLING) {
            // If the chain happens to be in the intermediate disabling state
            // in the same callback as the effect is in the intermediate enabling
            // state, the EffectProcessor should get the disabling signal, not the
            // enabling signal.
            effectiveEffectEnableState = EffectProcessor::DISABLING;
        } else if (chainEnableState == EffectProcessor::ENABLING) {
            // If the chain happens to be in the intermediate enabling state
            // in the same callback as the effect is in the intermediate disabling
            // state, the EffectProcessor should get the disabling signal, not the
            // enabling signal.
            if (effectiveEffectEnableState != EffectProcessor::DISABLING) {
                effectiveEffectEnableState = EffectProcessor::ENABLING;
            }
        }
    }

    bool processingOccured = false;

    if (effectiveEffectEnableState != EffectProcessor::DISABLED) {
        m_pProcessor->process(inputHandle, outputHandle, pInput, pOutput,
                              numSamples, sampleRate,
                              effectiveEffectEnableState, groupFeatures);

        processingOccured = true;

        if (!m_effectRampsFromDry) {
            // the effect does not fade, so we care for it
            if (effectiveEffectEnableState == EffectProcessor::DISABLING) {
                DEBUG_ASSERT(pInput != pOutput); // Fade to dry only works if pInput is not touched by pOutput
                // Fade out (fade to dry signal)
                SampleUtil::copy2WithRampingGain(pOutput,
                        pInput, 0.0, 1.0,
                        pOutput, 1.0, 0.0,
                        numSamples);
            } else if (effectiveEffectEnableState == EffectProcessor::ENABLING) {
                DEBUG_ASSERT(pInput != pOutput); // Fade to dry only works if pInput is not touched by pOutput
                // Fade in (fade to wet signal)
                SampleUtil::copy2WithRampingGain(pOutput,
                        pInput, 1.0, 0.0,
                        pOutput, 0.0, 1.0,
                        numSamples);
            }
        }
    }

    // Now that the EffectProcessor has been sent the intermediate enabling/disabling
    // signal, set the channel state to fully enabled/disabled for the next engine callback.
    EffectProcessor::EnableState& effectOnChannelState = m_effectEnableStateForChannelMatrix[inputHandle][outputHandle];
    if (effectOnChannelState == EffectProcessor::DISABLING) {
        effectOnChannelState = EffectProcessor::DISABLED;
    } else if (effectOnChannelState == EffectProcessor::ENABLING) {
        effectOnChannelState = EffectProcessor::ENABLED;
    }

    return processingOccured;
}
