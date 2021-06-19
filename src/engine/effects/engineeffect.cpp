#include "engine/effects/engineeffect.h"

#include "engine/engine.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffect::EngineEffect(EffectManifestPointer pManifest,
                           const QSet<ChannelHandleAndGroup>& activeInputChannels,
                           EffectsManager* pEffectsManager,
                           EffectInstantiatorPointer pInstantiator)
        : m_pManifest(pManifest),
          m_parameters(pManifest->parameters().size()),
          m_pEffectsManager(pEffectsManager) {
    const QList<EffectManifestParameterPointer>& parameters = m_pManifest->parameters();
    for (int i = 0; i < parameters.size(); ++i) {
        EffectManifestParameterPointer param = parameters.at(i);
        EngineEffectParameter* pParameter =
                new EngineEffectParameter(param);
        m_parameters[i] = pParameter;
        m_parametersById[param->id()] = pParameter;
    }

    for (const ChannelHandleAndGroup& inputChannel :
            pEffectsManager->registeredInputChannels()) {
        ChannelHandleMap<EffectEnableState> outputChannelMap;
        for (const ChannelHandleAndGroup& outputChannel :
                pEffectsManager->registeredOutputChannels()) {
            outputChannelMap.insert(outputChannel.handle(), EffectEnableState::Disabled);
        }
        m_effectEnableStateForChannelMatrix.insert(inputChannel.handle(), outputChannelMap);
    }

    // Creating the processor must come last.
    m_pProcessor = pInstantiator->instantiate(this, pManifest);
    //TODO: get actual configuration of engine
    const mixxx::EngineParameters bufferParameters(
          mixxx::audio::SampleRate(96000),
          MAX_BUFFER_LEN / mixxx::kEngineChannelCount);
    m_pProcessor->initialize(activeInputChannels, pEffectsManager, bufferParameters);
    m_effectRampsFromDry = pManifest->effectRampsFromDry();
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

EffectState* EngineEffect::createState(const mixxx::EngineParameters& bufferParameters) {
    if (!m_pProcessor) {
        return new EffectState(bufferParameters);
    }
    return m_pProcessor->createState(bufferParameters);
}

void EngineEffect::loadStatesForInputChannel(const ChannelHandle* inputChannel,
    EffectStatesMap* pStatesMap) {
    if (kEffectDebugOutput) {
        qDebug() << "EngineEffect::loadStatesForInputChannel" << this
                 << "loading states for input" << *inputChannel;
    }
    m_pProcessor->loadStatesForInputChannel(inputChannel, pStatesMap);
}

// Called from the main thread for garbage collection after an input channel is disabled
void EngineEffect::deleteStatesForInputChannel(const ChannelHandle* inputChannel) {
    m_pProcessor->deleteStatesForInputChannel(inputChannel);
}

bool EngineEffect::processEffectsRequest(EffectsRequest& message,
                                         EffectsResponsePipe* pResponsePipe) {
    EngineEffectParameter* pParameter = nullptr;
    EffectsResponse response(message);

    switch (message.type) {
        case EffectsRequest::SET_EFFECT_PARAMETERS:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "SET_EFFECT_PARAMETERS"
                         << "enabled" << message.SetEffectParameters.enabled;
            }

            for (auto& outputMap : m_effectEnableStateForChannelMatrix) {
                for (auto& enableState : outputMap) {
                    if (enableState != EffectEnableState::Disabled
                        && !message.SetEffectParameters.enabled) {
                        enableState = EffectEnableState::Disabling;
                    // If an input is not routed to the chain, and the effect gets
                    // a message to disable, then the effect gets the message to enable,
                    // process() will not have executed, so the enableState will still be
                    // DISABLING instead of DISABLED.
                    } else if ((enableState == EffectEnableState::Disabled ||
                               enableState == EffectEnableState::Disabling)
                               && message.SetEffectParameters.enabled) {
                        enableState = EffectEnableState::Enabling;
                    }
                }
            }

            response.success = true;
            pResponsePipe->writeMessage(response);
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
            pResponsePipe->writeMessage(response);
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
                           const EffectEnableState chainEnableState,
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

    EffectEnableState effectiveEffectEnableState =
        m_effectEnableStateForChannelMatrix[inputHandle][outputHandle];

    // If the EngineEffect is fully disabled, do not let
    // intermediate enabling/disabing signals from the chain override
    // the EngineEffect's state.
    if (effectiveEffectEnableState != EffectEnableState::Disabled) {
        if (chainEnableState == EffectEnableState::Disabled) {
            // If the chain is fully disabled, skip calling the EffectProcessor.
            effectiveEffectEnableState = EffectEnableState::Disabled;
        } else if (chainEnableState == EffectEnableState::Disabling) {
            // If the chain happens to be in the intermediate disabling state
            // in the same callback as the effect is in the intermediate enabling
            // state, the EffectProcessor should get the disabling signal, not the
            // enabling signal.
            effectiveEffectEnableState = EffectEnableState::Disabling;
        } else if (chainEnableState == EffectEnableState::Enabling) {
            // If the chain happens to be in the intermediate enabling state
            // in the same callback as the effect is in the intermediate disabling
            // state, the EffectProcessor should get the disabling signal, not the
            // enabling signal.
            if (effectiveEffectEnableState != EffectEnableState::Disabling) {
                effectiveEffectEnableState = EffectEnableState::Enabling;
            }
        }
    }

    bool processingOccured = false;

    if (effectiveEffectEnableState != EffectEnableState::Disabled) {
        //TODO: refactor rest of audio engine to use mixxx::AudioParameters
        const mixxx::EngineParameters bufferParameters(
              mixxx::audio::SampleRate(sampleRate),
              numSamples / mixxx::kEngineChannelCount);

        m_pProcessor->process(inputHandle, outputHandle, pInput, pOutput,
                              bufferParameters,
                              effectiveEffectEnableState, groupFeatures);

        processingOccured = true;

        if (!m_effectRampsFromDry) {
            // the effect does not fade, so we care for it
            if (effectiveEffectEnableState == EffectEnableState::Disabling) {
                DEBUG_ASSERT(pInput != pOutput); // Fade to dry only works if pInput is not touched by pOutput
                // Fade out (fade to dry signal)
                SampleUtil::linearCrossfadeBuffersOut(
                        pOutput,
                        pInput,
                        numSamples);
            } else if (effectiveEffectEnableState == EffectEnableState::Enabling) {
                DEBUG_ASSERT(pInput != pOutput); // Fade to dry only works if pInput is not touched by pOutput
                // Fade in (fade to wet signal)
                SampleUtil::linearCrossfadeBuffersIn(
                        pOutput,
                        pInput,
                        numSamples);
            }
        }
    }

    // Now that the EffectProcessor has been sent the intermediate enabling/disabling
    // signal, set the channel state to fully enabled/disabled for the next engine callback.
    EffectEnableState& effectOnChannelState = m_effectEnableStateForChannelMatrix[inputHandle][outputHandle];
    if (effectOnChannelState == EffectEnableState::Disabling) {
        effectOnChannelState = EffectEnableState::Disabled;
    } else if (effectOnChannelState == EffectEnableState::Enabling) {
        effectOnChannelState = EffectEnableState::Enabled;
    }

    return processingOccured;
}
