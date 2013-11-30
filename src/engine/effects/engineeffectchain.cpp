#include "engine/effects/engineeffectchain.h"

#include "engine/effects/engineeffect.h"

EngineEffectChain::EngineEffectChain(const QString& id)
        : m_id(id),
          m_bEnabled(false),
          m_dMix(0),
          m_dParameter(0) {
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
    m_dMix = message.SetEffectChainParameters.mix;
    m_dParameter = message.SetEffectChainParameters.parameter;
    return true;
}

bool EngineEffectChain::processEffectsRequest(const EffectsRequest& message,
                                              EffectsResponsePipe* pResponsePipe) {
    if (message.targetId != m_id) {
        return false;
    }

    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            response.success = addEffect(message.AddEffectToChain.pEffect,
                                         message.AddEffectToChain.iIndex);
            break;
        case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            response.success = removeEffect(
                message.RemoveEffectFromChain.pEffect);
            break;
        case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
            response.success = updateParameters(message);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

void EngineEffectChain::process(const QString channelId,
                                const CSAMPLE* pInput, CSAMPLE* pOutput,
                                const unsigned int numSamples) {
}
