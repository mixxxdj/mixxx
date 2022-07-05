#include "engine/effects/engineeffect.h"

#include "engine/engine.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffect::EngineEffect(EffectManifestPointer pManifest,
        EffectsBackendManagerPointer pBackendManager,
        const QSet<GroupHandle>& activeInputChannels,
        const QSet<GroupHandle>& registeredInputChannels,
        const QSet<GroupHandle>& registeredOutputChannels)
        : m_pManifest(pManifest),
          m_pProcessor(pBackendManager->createProcessor(pManifest)),
          m_parameters(pManifest->parameters().size()) {
    const QList<EffectManifestParameterPointer>& parameters = m_pManifest->parameters();
    for (int i = 0; i < parameters.size(); ++i) {
        EffectManifestParameterPointer param = parameters.at(i);
        EngineEffectParameterPointer pParameter(new EngineEffectParameter(param));
        m_parameters[i] = pParameter;
        m_parametersById[param->id()] = pParameter;
    }

    for (GroupHandle inputChannel : registeredInputChannels) {
        ChannelHandleMap<EffectEnableState> outputChannelMap;
        for (GroupHandle outputChannel : registeredOutputChannels) {
            outputChannelMap.insert(outputChannel, EffectEnableState::Disabled);
        }
        m_effectEnableStateForChannelMatrix.insert(inputChannel, outputChannelMap);
    }

    m_pProcessor->loadEngineEffectParameters(m_parametersById);

    //TODO: get actual configuration of engine
    const mixxx::EngineParameters engineParameters(
            mixxx::audio::SampleRate(96000),
            MAX_BUFFER_LEN / mixxx::kEngineChannelCount);
    m_pProcessor->initialize(activeInputChannels, registeredOutputChannels, engineParameters);
    m_effectRampsFromDry = pManifest->effectRampsFromDry();
}

EngineEffect::~EngineEffect() {
    if (kEffectDebugOutput) {
        qDebug() << debugString() << "destroyed";
    }
    m_parametersById.clear();
    m_parameters.clear();
}

EffectState* EngineEffect::createState(const mixxx::EngineParameters& engineParameters) {
    VERIFY_OR_DEBUG_ASSERT(m_pProcessor) {
        return new EffectState(engineParameters);
    }
    return m_pProcessor->createState(engineParameters);
}

void EngineEffect::loadStatesForInputChannel(GroupHandle inputChannel,
        EffectStatesMap* pStatesMap) {
    if (kEffectDebugOutput) {
        qDebug() << "EngineEffect::loadStatesForInputChannel" << this
                 << "loading states for input" << inputChannel;
    }
    m_pProcessor->loadStatesForInputChannel(inputChannel, pStatesMap);
}

void EngineEffect::deleteStatesForInputChannel(GroupHandle inputChannel) {
    m_pProcessor->deleteStatesForInputChannel(inputChannel);
}

bool EngineEffect::processEffectsRequest(EffectsRequest& message,
                                         EffectsResponsePipe* pResponsePipe) {
    EngineEffectParameterPointer pParameter;
    EffectsResponse response(message);

    switch (message.type) {
    case EffectsRequest::SET_EFFECT_PARAMETERS:
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "SET_EFFECT_PARAMETERS"
                     << "enabled" << message.SetEffectParameters.enabled;
        }

        for (auto& outputMap : m_effectEnableStateForChannelMatrix) {
            for (auto& enableState : outputMap) {
                if (enableState != EffectEnableState::Disabled &&
                        !message.SetEffectParameters.enabled) {
                    enableState = EffectEnableState::Disabling;
                    // If an input is not routed to the chain, and the effect gets
                    // a message to disable, then the effect gets the message to enable,
                    // process() will not have executed, so the enableState will still be
                    // DISABLING instead of DISABLED.
                } else if ((enableState == EffectEnableState::Disabled ||
                                   enableState ==
                                           EffectEnableState::Disabling) &&
                        message.SetEffectParameters.enabled) {
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
                     << "value" << message.value;
        }
        pParameter = m_parameters.value(
                message.SetParameterParameters.iParameter, EngineEffectParameterPointer());
        if (pParameter) {
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

bool EngineEffect::process(
        GroupHandle inputHandle,
        GroupHandle outputHandle,
        const CSAMPLE* pInput,
        CSAMPLE* pOutput,
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
    // intermediate enabling/disabling signals from the chain override
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
        const mixxx::EngineParameters engineParameters(
                mixxx::audio::SampleRate(sampleRate),
                numSamples / mixxx::kEngineChannelCount);

        m_pProcessor->process(inputHandle,
                outputHandle,
                pInput,
                pOutput,
                engineParameters,
                effectiveEffectEnableState,
                groupFeatures);

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
    EffectEnableState& effectOnChannelState =
            m_effectEnableStateForChannelMatrix[inputHandle][outputHandle];
    if (effectOnChannelState == EffectEnableState::Disabling) {
        effectOnChannelState = EffectEnableState::Disabled;
    } else if (effectOnChannelState == EffectEnableState::Enabling) {
        effectOnChannelState = EffectEnableState::Enabled;
    }

    return processingOccured;
}
