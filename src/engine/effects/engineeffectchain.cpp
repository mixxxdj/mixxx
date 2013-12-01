#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"
#include "sampleutil.h"

EngineEffectChain::EngineEffectChain(const QString& id)
        : m_id(id),
          m_bEnabled(false),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_dParameter(0),
          m_pBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)) {
    // Make sure there's plenty of room so we don't allocate on
    // insertion/deletion.
    m_enabledGroups.reserve(64);
}

EngineEffectChain::~EngineEffectChain() {
}

bool EngineEffectChain::addEffect(EngineEffect* pEffect, int iIndex) {
    if (iIndex < 0 || iIndex > m_effects.size()) {
        qDebug() << debugString()
                 << "WARNING: ADD_EFFECT_TO_CHAIN message with invalid index:"
                 << iIndex;
    }
    if (m_effects.contains(pEffect)) {
        qDebug() << debugString() << "WARNING: effect already added to EngineEffectChain:"
                 << pEffect->name();
        return false;
    }
    m_effects.insert(iIndex, pEffect);
    return true;
}

bool EngineEffectChain::removeEffect(EngineEffect* pEffect) {
    return m_effects.removeAll(pEffect) > 0;
}

bool EngineEffectChain::updateParameters(const EffectsRequest& message) {
    // TODO(rryan): Parameter interpolation.
    m_bEnabled = message.SetEffectChainParameters.enabled;
    m_insertionType = message.SetEffectChainParameters.insertion_type;
    m_dMix = message.SetEffectChainParameters.mix;
    m_dParameter = message.SetEffectChainParameters.parameter;
    return true;
}

bool EngineEffectChain::processEffectsRequest(const EffectsRequest& message,
                                              EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            qDebug() << debugString() << "ADD_EFFECT_TO_CHAIN"
                     << message.AddEffectToChain.pEffect
                     << message.AddEffectToChain.iIndex;
            response.success = addEffect(message.AddEffectToChain.pEffect,
                                         message.AddEffectToChain.iIndex);
            break;
        case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            qDebug() << debugString() << "REMOVE_EFFECT_FROM_CHAIN"
                     << message.RemoveEffectFromChain.pEffect;
            response.success = removeEffect(
                message.RemoveEffectFromChain.pEffect);
            break;
        case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
            qDebug() << debugString() << "SET_EFFECT_CHAIN_PARAMETERS"
                     << "enabled" << message.SetEffectChainParameters.enabled
                     << "mix" << message.SetEffectChainParameters.mix
                     << "parameter" << message.SetEffectChainParameters.parameter;
            response.success = updateParameters(message);
            break;
        case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_GROUP:
            qDebug() << debugString() << "ENABLE_EFFECT_CHAIN_FOR_GROUP"
                     << message.group;
            response.success = enableForGroup(message.group);
            break;
        case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_GROUP:
            qDebug() << debugString() << "DISABLE_EFFECT_CHAIN_FOR_GROUP"
                     << message.group;
            response.success = disableForGroup(message.group);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

bool EngineEffectChain::enabledForGroup(const QString& group) const {
    return m_enabledGroups.contains(group);
}

bool EngineEffectChain::enableForGroup(const QString& group) {
    m_enabledGroups.insert(group);
    return true;
}

bool EngineEffectChain::disableForGroup(const QString& group) {
    m_enabledGroups.remove(group);
    return true;
}

void EngineEffectChain::process(const QString& group,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples) {

    CSAMPLE wet_gain = m_dMix;
    CSAMPLE dry_gain = 1 - m_dMix;

    // INSERT mode: output = input * (1-wet) + effect(input) * wet
    if (m_insertionType == EffectChain::INSERT) {
        // Fully wet, insert optimization. No temporary buffer needed.
        if (wet_gain == 1.0) {
            for (int i = 0; i < m_effects.size(); ++i) {
                EngineEffect* pEffect = m_effects[i];
                const CSAMPLE* pIntermediateInput = (i == 0) ? pInput : pOutput;
                CSAMPLE* pIntermediateOutput = pOutput;
                pEffect->process(group, pIntermediateInput, pIntermediateOutput, numSamples);
            }
        } else {
            // Clear scratch buffer.
            SampleUtil::applyGain(m_pBuffer, 0.0, numSamples);

            // Chain each effect
            for (int i = 0; i < m_effects.size(); ++i) {
                EngineEffect* pEffect = m_effects[i];
                const CSAMPLE* pIntermediateInput = (i == 0) ? pInput : m_pBuffer;
                CSAMPLE* pIntermediateOutput = m_pBuffer;
                pEffect->process(group, pIntermediateInput, pIntermediateOutput, numSamples);
            }

            // m_pBuffer now contains the fully wet output.
            // TODO(rryan): benchmark applyGain followed by addWithGain versus
            // copy2WithGain.
            SampleUtil::copy2WithGain(pOutput, pInput, dry_gain,
                                      m_pBuffer, wet_gain, numSamples);
        }
    } else { // SEND mode: output = input * (2-wet) + effect(input) * wet
        // Clear scratch buffer.
        SampleUtil::applyGain(m_pBuffer, 0.0, numSamples);

        // Chain each effect
        for (int i = 0; i < m_effects.size(); ++i) {
            EngineEffect* pEffect = m_effects[i];
            const CSAMPLE* pIntermediateInput = (i == 0) ? pInput : m_pBuffer;
            CSAMPLE* pIntermediateOutput = m_pBuffer;
            pEffect->process(group, pIntermediateInput, pIntermediateOutput, numSamples);
        }

        // m_pBuffer now contains the fully wet output.
        if (pInput == pOutput) {
            SampleUtil::add2WithGain(pOutput, pInput, dry_gain,
                                     m_pBuffer, wet_gain, numSamples);
        } else {
            SampleUtil::copy2WithGain(pOutput, pInput, 1.0 + dry_gain,
                                      m_pBuffer, wet_gain, numSamples);
        }
    }
}
