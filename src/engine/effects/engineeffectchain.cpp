#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"
#include "sampleutil.h"
#include "util/defs.h"

EngineEffectChain::EngineEffectChain(const QString& id)
        : m_id(id),
          m_bEnabled(true),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
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

bool EngineEffectChain::updateParameters(const EffectsRequest& message) {
    // TODO(rryan): Parameter interpolation.
    bool wasEnabled = m_bEnabled;
    m_bEnabled = message.SetEffectChainParameters.enabled;
    m_insertionType = message.SetEffectChainParameters.insertion_type;
    m_dMix = message.SetEffectChainParameters.mix;

    // If our enabled state changed then tell each group to ramp in or out.
    if (wasEnabled ^ m_bEnabled) {
        for (QMap<QString, GroupStatus>::iterator it = m_groupStatus.begin();
             it != m_groupStatus.end(); ++it) {
            GroupStatus& status = it.value();

            if (m_bEnabled) {
                // Ramp in.
                status.old_gain = 0;
            } else {
                // Ramp out.
                status.ramp_out = true;
            }
        }
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

bool EngineEffectChain::enabledForGroup(const QString& group) const {
    const GroupStatus& status = m_groupStatus[group];
    return status.enabled;
}

bool EngineEffectChain::enableForGroup(const QString& group) {
    GroupStatus& status = m_groupStatus[group];
    status.enabled = true;
    // Ramp in to prevent clicking.
    status.old_gain = 0;
    status.ramp_out = false;
    return true;
}

bool EngineEffectChain::disableForGroup(const QString& group) {
    GroupStatus& status = m_groupStatus[group];
    status.enabled = false;
    // Ramp out to prevent clicking.
    status.ramp_out = true;
    return true;
}

void EngineEffectChain::process(const QString& group,
                                CSAMPLE* pInOut,
                                const unsigned int numSamples,
                                const GroupFeatureState& groupFeatures) {
    GroupStatus& group_info = m_groupStatus[group];
    bool bEnabled = m_bEnabled && group_info.enabled;

    // If the chain is not enabled and the group is not enabled and we are not
    // ramping out then do nothing.
    if (!bEnabled && !group_info.ramp_out) {
        return;
    }

    // At this point either the chain and group are enabled or we are ramping
    // out. If we are ramping out then ramp to 0 instead of m_dMix.
    CSAMPLE wet_gain = group_info.ramp_out ? 0 : m_dMix;
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
                                 numSamples, groupFeatures);
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
                                 numSamples, groupFeatures);
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
                             pIntermediateOutput, numSamples, groupFeatures);
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
    group_info.ramp_out = false;
}
