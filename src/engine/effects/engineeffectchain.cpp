#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffectChain::EngineEffectChain(const QString& id,
                                     const QSet<ChannelHandleAndGroup>& registeredInputChannels,
                                     const QSet<ChannelHandleAndGroup>& registeredOutputChannels)
        : m_id(id),
          m_enableState(EffectProcessor::ENABLED),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_buffer1(MAX_BUFFER_LEN),
          m_buffer2(MAX_BUFFER_LEN) {
    // Try to prevent memory allocation.
    m_effects.reserve(256);

    for (const ChannelHandleAndGroup& inputChannel : registeredInputChannels) {
        ChannelHandleMap<ChannelStatus> outputChannelMap;
        for (const ChannelHandleAndGroup& outputChannel : registeredOutputChannels) {
            outputChannelMap.insert(outputChannel.handle(), ChannelStatus());
        }
        m_chainStatusForChannelMatrix.insert(inputChannel.handle(), outputChannelMap);
    }
}

EngineEffectChain::~EngineEffectChain() {
}

bool EngineEffectChain::addEffect(EngineEffect* pEffect, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: ADD_EFFECT_TO_CHAIN message with invalid index:"
                     << iIndex;
        }
        return false;
    }
    if (m_effects.contains(pEffect)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: effect already added to EngineEffectChain:"
                     << pEffect->name();
        }
        return false;
    }

    while (iIndex >= m_effects.size()) {
        m_effects.append(NULL);
    }
    m_effects.replace(iIndex, pEffect);
    return true;
}

bool EngineEffectChain::removeEffect(EngineEffect* pEffect, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: REMOVE_EFFECT_FROM_CHAIN message with invalid index:"
                     << iIndex;
        }
        return false;
    }
    if (m_effects.at(iIndex) != pEffect) {
        qDebug() << debugString()
                 << "WARNING: REMOVE_EFFECT_FROM_CHAIN consistency error"
                 << m_effects.at(iIndex) << "loaded but received request to remove"
                 << pEffect;
        return false;
    }

    m_effects.replace(iIndex, NULL);
    return true;
}

// this is called from the engine thread onCallbackStart()
bool EngineEffectChain::updateParameters(const EffectsRequest& message) {
    // TODO(rryan): Parameter interpolation.
    m_insertionType = message.SetEffectChainParameters.insertion_type;
    m_dMix = message.SetEffectChainParameters.mix;

    if (m_enableState != EffectProcessor::DISABLED && !message.SetEffectParameters.enabled) {
        m_enableState = EffectProcessor::DISABLING;
    } else if (m_enableState == EffectProcessor::DISABLED && message.SetEffectParameters.enabled) {
        m_enableState = EffectProcessor::ENABLING;
    }
    return true;
}

bool EngineEffectChain::processEffectsRequest(const EffectsRequest& message,
                                              EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ADD_EFFECT_TO_CHAIN"
                         << message.AddEffectToChain.pEffect
                         << message.AddEffectToChain.iIndex;
            }
            response.success = addEffect(message.AddEffectToChain.pEffect,
                                         message.AddEffectToChain.iIndex);
            break;
        case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "REMOVE_EFFECT_FROM_CHAIN"
                         << message.RemoveEffectFromChain.pEffect
                         << message.RemoveEffectFromChain.iIndex;
            }
            response.success = removeEffect(message.RemoveEffectFromChain.pEffect,
                                            message.RemoveEffectFromChain.iIndex);
            break;
        case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "SET_EFFECT_CHAIN_PARAMETERS"
                         << "enabled" << message.SetEffectChainParameters.enabled
                         << "mix" << message.SetEffectChainParameters.mix;
            }
            response.success = updateParameters(message);
            break;
        case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ENABLE_EFFECT_CHAIN_FOR_CHANNEL"
                         << message.channel;
            }
            response.success = enableForInputChannel(message.channel);
            break;
        case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "DISABLE_EFFECT_CHAIN_FOR_CHANNEL"
                         << message.channel;
            }
            response.success = disableForInputChannel(message.channel);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

bool EngineEffectChain::enableForInputChannel(const ChannelHandle& inputHandle) {
    auto& outputMap = m_chainStatusForChannelMatrix[inputHandle];
    for (auto& outputChannelStatus : outputMap) {
        if (outputChannelStatus.enable_state != EffectProcessor::ENABLED) {
            outputChannelStatus.enable_state = EffectProcessor::ENABLING;
        }
    }
    return true;
}

bool EngineEffectChain::disableForInputChannel(const ChannelHandle& inputHandle) {
    auto& outputMap = m_chainStatusForChannelMatrix[inputHandle];
    for (auto& outputChannelStatus : outputMap) {
        if (outputChannelStatus.enable_state != EffectProcessor::DISABLED) {
            outputChannelStatus.enable_state = EffectProcessor::DISABLING;
        }
    }
    return true;
}

EngineEffectChain::ChannelStatus& EngineEffectChain::getChannelStatus(
        const ChannelHandle& inputHandle,
        const ChannelHandle& outputHandle) {
    ChannelStatus& status = m_chainStatusForChannelMatrix[inputHandle][outputHandle];
    return status;
}

bool EngineEffectChain::process(const ChannelHandle& inputHandle,
                                const ChannelHandle& outputHandle,
                                CSAMPLE* pIn, CSAMPLE* pOut,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const GroupFeatureState& groupFeatures) {
    // Compute the effective enable state from the channel input routing switch and
    // the chain's enable state. When either of these are turned on/off, send the
    // effects the intermediate enabling/disabling signal.
    // If the EngineEffect is not disabled for the channel, it will pass the
    // intermediate state down to the EffectProcessor, which is then responsible for reacting
    // appropriately, for example the Echo effect clears its internal buffer for the channel
    // when it gets the intermediate disabling signal.

    ChannelStatus& channelStatus = m_chainStatusForChannelMatrix[inputHandle][outputHandle];
    EffectProcessor::EnableState effectiveChainEnableState = channelStatus.enable_state;

    if (m_enableState != EffectProcessor::ENABLED) {
        effectiveChainEnableState = m_enableState;
    }

    CSAMPLE currentWetGain = m_dMix;
    CSAMPLE lastCallbackWetGain = channelStatus.old_gain;

    bool processingOccured = false;
    if (effectiveChainEnableState != EffectProcessor::DISABLED) {
        // Ramping code inside the effects need to access the original samples
        // after writing to the output buffer. This requires not to use the same buffer
        // for in and output: Also, ChannelMixer::applyEffectsAndMixChannels(Ramping)
        // requires that the input buffer does not get modified.
        bool processingOccured = false;
        CSAMPLE* pIntermediateInput = pIn;
        CSAMPLE* pIntermediateOutput = m_buffer1.data();

        for (EngineEffect* pEffect: m_effects) {
            if (pEffect != nullptr) {
                if (pIntermediateInput == m_buffer1.data()) {
                    pIntermediateOutput = m_buffer2.data();
                } else {
                    pIntermediateOutput = m_buffer1.data();
                }

                if (pEffect->process(inputHandle, outputHandle,
                                     pIntermediateInput, pIntermediateOutput,
                                     numSamples, sampleRate,
                                     effectiveChainEnableState, groupFeatures)) {
                    processingOccured = true;
                    pIntermediateInput = pIntermediateOutput;
                }
            }
        }

        if (processingOccured) {
            // pIntermediateInput is the output of the last processed effect. It would be the
            // intermediate input of the next effect if there was one.
            if (m_insertionType == EffectChain::INSERT) {
                // INSERT mode: output = input * (1-wet) + effect(input) * wet
                SampleUtil::copy2WithRampingGain(
                        pOut,
                        pIn, 1.0 - lastCallbackWetGain, 1.0 - currentWetGain,
                        pIntermediateInput, lastCallbackWetGain, currentWetGain,
                        numSamples);
            } else {
                // SEND mode: output = input + effect(input) * wet
                SampleUtil::copy2WithRampingGain(
                        pOut,
                        pIn, 1.0, 1.0,
                        pIntermediateInput, lastCallbackWetGain, currentWetGain,
                        numSamples);
            }
        }
    }

    channelStatus.old_gain = currentWetGain;

    // If the EffectProcessors have been sent a signal for the intermediate
    // enabling/disabling state, set the channel state or chain state
    // to the fully enabled/disabled state for the next engine callback.

    EffectProcessor::EnableState& chainOnChannelEnableState = channelStatus.enable_state;
    if (chainOnChannelEnableState == EffectProcessor::DISABLING) {
        chainOnChannelEnableState = EffectProcessor::DISABLED;
    } else if (chainOnChannelEnableState == EffectProcessor::ENABLING) {
        chainOnChannelEnableState = EffectProcessor::ENABLED;
    }

    if (m_enableState == EffectProcessor::DISABLING) {
        m_enableState = EffectProcessor::DISABLED;
    } else if (m_enableState == EffectProcessor::ENABLING) {
        m_enableState = EffectProcessor::ENABLED;
    }

    return processingOccured;
}
