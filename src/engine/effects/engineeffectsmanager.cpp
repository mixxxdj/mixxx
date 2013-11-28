#include "engine/effects/engineeffectsmanager.h"

#include "engine/effects/engineeffectchain.h"

EngineEffectsManager::EngineEffectsManager(EffectsResponsePipe* pResponsePipe)
        : m_pResponsePipe(pResponsePipe) {
}

EngineEffectsManager::~EngineEffectsManager() {
}

void EngineEffectsManager::onCallbackStart() {
}

void EngineEffectsManager::process(const QString channelId,
                                   const CSAMPLE* pInput, CSAMPLE* pOutput,
                                   const unsigned int numSamples) {
}

bool EngineEffectsManager::addEffectChain(EngineEffectChain* pChain) {
    if (m_chains.contains(pChain)) {
        qDebug() << debugString() << "WARNING: EffectChain already added to EngineEffectsManager:"
                 << pChain->id();
        return false;
    }
    m_chains.append(pChain);
    return true;
}

bool EngineEffectsManager::removeEffectChain(EngineEffectChain* pChain) {
    return m_chains.removeAll(pChain) > 0;
}

bool EngineEffectsManager::processEffectsRequest(const EffectsRequest& message,
                                                 const QSharedPointer<EffectsResponsePipe>& pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_EFFECT_CHAIN:
            response.success = addEffectChain(message.AddEffectChain.pChain);
            break;
        case EffectsRequest::REMOVE_EFFECT_CHAIN:
            response.success = removeEffectChain(message.AddEffectChain.pChain);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}
