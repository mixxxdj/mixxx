#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"
#include "util/defs.h"
#include "util/sample.h"

EngineEffectChain::EngineEffectChain(const QString& id)
        : m_id(id),
          m_enableState(EffectProcessor::ENABLED),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_buffer1(MAX_BUFFER_LEN),
          m_buffer2(MAX_BUFFER_LEN) {
    // Try to prevent memory allocation.
    m_effects.reserve(256);
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
        case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_CHANNEL:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ENABLE_EFFECT_CHAIN_FOR_CHANNEL"
                         << message.channel;
            }
            response.success = enableForChannel(message.channel);
            break;
        case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_CHANNEL:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "DISABLE_EFFECT_CHAIN_FOR_CHANNEL"
                         << message.channel;
            }
            response.success = disableForChannel(message.channel);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

bool EngineEffectChain::enableForChannel(const ChannelHandle& handle) {
    ChannelStatus& status = getChannelStatus(handle);
    if (status.enable_state != EffectProcessor::ENABLED) {
        status.enable_state = EffectProcessor::ENABLING;
    }
    return true;
}

bool EngineEffectChain::disableForChannel(const ChannelHandle& handle) {
    ChannelStatus& status = getChannelStatus(handle);
    if (status.enable_state != EffectProcessor::DISABLED) {
        status.enable_state = EffectProcessor::DISABLING;
    }
    return true;
}

EngineEffectChain::ChannelStatus& EngineEffectChain::getChannelStatus(
        const ChannelHandle& handle) {
    return m_channelStatus[handle];
}

void EngineEffectChain::process(const ChannelHandle& handle,
                                CSAMPLE* pInOut,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const GroupFeatureState& groupFeatures) {
    ChannelStatus& channel_info = getChannelStatus(handle);

    if (m_enableState == EffectProcessor::DISABLED
            || channel_info.enable_state == EffectProcessor::DISABLED) {
        // If the chain is not enabled and the channel is not enabled and we are not
        // ramping out then do nothing.
        return;
    }

    EffectProcessor::EnableState effectiveEnableState = channel_info.enable_state;

    if (channel_info.enable_state == EffectProcessor::DISABLING) {
        channel_info.enable_state = EffectProcessor::DISABLED;
    } else if (channel_info.enable_state == EffectProcessor::ENABLING) {
        channel_info.enable_state = EffectProcessor::ENABLED;
    }

    if (m_enableState == EffectProcessor::DISABLING) {
        effectiveEnableState = EffectProcessor::DISABLING;
        m_enableState = EffectProcessor::DISABLED;
    } else if (m_enableState == EffectProcessor::ENABLING) {
        effectiveEnableState = EffectProcessor::ENABLING;
        m_enableState = EffectProcessor::ENABLED;
    }

    // At this point either the chain and channel are enabled or we are ramping
    // out. If we are ramping out then ramp to 0 instead of m_dMix.
    CSAMPLE wet_gain = m_dMix;
    CSAMPLE wet_gain_old = channel_info.old_gain;

    if (wet_gain_old != 0.0 && wet_gain == 0.0) {
        // Tell the effects that this is the last call before disabling
        effectiveEnableState = EffectProcessor::DISABLING;
    }

    // Ramping code inside the effects need to access the original samples
    // after writing to the output buffer. This requires not to use the same buffer
    // for in and output:
    int enabledEffectCount = 0;
    CSAMPLE* pIntermediateInput = pInOut;
    CSAMPLE* pIntermediateOutput = m_buffer1.data();

    for (EngineEffect* pEffect: m_effects) {
        if (pEffect == nullptr || pEffect->disabled()) {
            continue;
        }
        pEffect->process(
                handle,
                pIntermediateInput, pIntermediateOutput,
                numSamples, sampleRate,
                effectiveEnableState, groupFeatures);

        ++enabledEffectCount;
        if (enabledEffectCount % 2) {
            pIntermediateInput = m_buffer1.data();
            pIntermediateOutput = m_buffer2.data();
        } else {
            pIntermediateInput = m_buffer2.data();
            pIntermediateOutput = m_buffer1.data();
        }
    }

    // Mix the effected signal, unless no effects are enabled
    // or the chain is fully dry and not ramping.
    if (enabledEffectCount > 0 && !(wet_gain == 0.0 && wet_gain_old == 0.0)) {
        if (m_insertionType == EffectChain::INSERT) {
            // INSERT mode: output = input * (1-wet) + effect(input) * wet
            SampleUtil::copy2WithRampingGain(
                    pInOut,
                    pInOut, 1.0 - wet_gain_old, 1.0 - wet_gain,
                    pIntermediateInput, wet_gain_old, wet_gain,
                    numSamples);
        } else {
            // SEND mode: output = input + effect(input) * wet
            SampleUtil::addWithRampingGain(
                    pInOut,
                    pIntermediateInput, wet_gain_old, wet_gain,
                    numSamples);
        }
    }

    // Update ChannelStatus with the latest values.
    channel_info.old_gain = wet_gain;
}
