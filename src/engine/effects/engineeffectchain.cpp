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

bool EngineEffectChain::addEffect(EngineEffect* pEffect) {
    if (m_effects.contains(pEffect)) {
        qDebug() << debugString() << "WARNING: effect already added to EngineEffectChain:"
                 << pEffect->name();
        return false;
    }
    m_effects.append(pEffect);
    return true;
}

bool EngineEffectChain::removeEffect(EngineEffect* pEffect) {
    return m_effects.removeAll(pEffect) > 0;
}

bool EngineEffectChain::processEffectsRequest(const EffectsRequest& message,
                                              const QSharedPointer<EffectsResponsePipe>& pResponsePipe) {
    EffectsResponse response(message);
    if (message.chainId != m_id) {
        return false;
    }

    switch (message.type) {
        case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            response.success = addEffect(
                message.AddEffectToChain.pEffect);
            break;
        case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            response.success = removeEffect(
                message.RemoveEffectFromChain.pEffect);
            break;
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
