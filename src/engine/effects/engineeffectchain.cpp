#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"
#include "sampleutil.h"
#include "util/defs.h"

EngineEffectChain::EngineEffectChain(const QString& id)
        : m_id(id),
          m_enableState(EffectProcessor::ENABLED),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    // Try to prevent memory allocation.
    m_effects.reserve(256);
}

EngineEffectChain::~EngineEffectChain() {
    SampleUtil::free(m_pBuffer);
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
        case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_GROUP:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ENABLE_EFFECT_CHAIN_FOR_GROUP"
                         << message.group;
            }
            response.success = enableForGroup(message.group);
            break;
        case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_GROUP:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "DISABLE_EFFECT_CHAIN_FOR_GROUP"
                         << message.group;
            }
            response.success = disableForGroup(message.group);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

bool EngineEffectChain::enableForGroup(const QString& group) {
    GroupStatus& status = m_groupStatus[group];
    if (status.enable_state != EffectProcessor::ENABLED) {
        status.enable_state = EffectProcessor::ENABLING;
    }
    return true;
}

bool EngineEffectChain::disableForGroup(const QString& group) {
    GroupStatus& status = m_groupStatus[group];
    if (status.enable_state != EffectProcessor::DISABLED) {
        status.enable_state = EffectProcessor::DISABLING;
    }
    return true;
}

void EngineEffectChain::process(const QString& group,
                                CSAMPLE* pInOut,
                                const unsigned int numSamples,
                                const unsigned int sampleRate,
                                const GroupFeatureState& groupFeatures) {
    GroupStatus& group_info = m_groupStatus[group];

    if (m_enableState == EffectProcessor::DISABLED
            || group_info.enable_state == EffectProcessor::DISABLED) {
        // If the chain is not enabled and the group is not enabled and we are not
        // ramping out then do nothing.
        return;
    }

    EffectProcessor::EnableState effectiveEnableState = group_info.enable_state;

    if (m_enableState == EffectProcessor::DISABLING) {
        effectiveEnableState = EffectProcessor::DISABLING;
    } else if (m_enableState == EffectProcessor::ENABLING) {
        effectiveEnableState = EffectProcessor::ENABLING;
    }

    // At this point either the chain and group are enabled or we are ramping
    // out. If we are ramping out then ramp to 0 instead of m_dMix.
    CSAMPLE wet_gain = m_dMix;
    CSAMPLE wet_gain_old = group_info.old_gain;

    // INSERT mode: output = input * (1-wet) + effect(input) * wet
    if (m_insertionType == EffectChain::INSERT) {
        if (wet_gain_old == 1.0 && wet_gain == 1.0) {
            // Fully wet, no ramp, insert optimization. No temporary buffer needed.
            for (int i = 0; i < m_effects.size(); ++i) {
                EngineEffect* pEffect = m_effects[i];
                if (pEffect == NULL || !pEffect->enabled()) {
                    continue;
                }
                pEffect->process(group, pInOut, pInOut,
                                 numSamples, sampleRate,
                                 effectiveEnableState, groupFeatures);
            }
        } else if (wet_gain_old == 0.0 && wet_gain == 0.0) {
            // Fully dry, no ramp, insert optimization. No action is needed
        } else {
            // Clear scratch buffer.
            SampleUtil::clear(m_pBuffer, numSamples);

            // Chain each effect
            bool anyProcessed = false;
            for (int i = 0; i < m_effects.size(); ++i) {
                EngineEffect* pEffect = m_effects[i];
                if (pEffect == NULL || !pEffect->enabled()) {
                    continue;
                }
                const CSAMPLE* pIntermediateInput = (i == 0) ? pInOut : m_pBuffer;
                CSAMPLE* pIntermediateOutput = m_pBuffer;
                pEffect->process(group, pIntermediateInput, pIntermediateOutput,
                                 numSamples, sampleRate,
                                 effectiveEnableState, groupFeatures);
                anyProcessed = true;
            }

            if (anyProcessed) {
                // m_pBuffer now contains the fully wet output.
                // TODO(rryan): benchmark applyGain followed by addWithGain versus
                // copy2WithGain.
                SampleUtil::copy2WithRampingGain(
                    pInOut, pInOut, 1.0 - wet_gain_old, 1.0 - wet_gain,
                    m_pBuffer, wet_gain_old, wet_gain, numSamples);
            }
        }
    } else { // SEND mode: output = input + effect(input) * wet
        // Clear scratch buffer.
        SampleUtil::applyGain(m_pBuffer, 0.0, numSamples);

        // Chain each effect
        bool anyProcessed = false;
        for (int i = 0; i < m_effects.size(); ++i) {
            EngineEffect* pEffect = m_effects[i];
            if (pEffect == NULL || !pEffect->enabled()) {
                continue;
            }
            const CSAMPLE* pIntermediateInput = (i == 0) ? pInOut : m_pBuffer;
            CSAMPLE* pIntermediateOutput = m_pBuffer;
            pEffect->process(group, pIntermediateInput,
                             pIntermediateOutput, numSamples, sampleRate,
                             effectiveEnableState, groupFeatures);
            anyProcessed = true;
        }

        if (anyProcessed) {
            // m_pBuffer now contains the fully wet output.
            SampleUtil::addWithRampingGain(pInOut, m_pBuffer,
                                           wet_gain_old, wet_gain, numSamples);
        }
    }

    // Update GroupStatus with the latest values.
    group_info.old_gain = wet_gain;

    if (m_enableState == EffectProcessor::DISABLING) {
        m_enableState = EffectProcessor::DISABLED;
    } else if (m_enableState == EffectProcessor::ENABLING) {
        m_enableState = EffectProcessor::ENABLED;
    }

    if (group_info.enable_state == EffectProcessor::DISABLING) {
        group_info.enable_state = EffectProcessor::DISABLED;
    } else if (group_info.enable_state == EffectProcessor::ENABLING) {
        group_info.enable_state = EffectProcessor::ENABLED;
    }
}
