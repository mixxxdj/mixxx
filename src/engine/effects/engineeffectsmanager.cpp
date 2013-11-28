#include "engine/effects/engineeffectsmanager.h"

#include "engine/effects/engineeffectchain.h"

EngineEffectsManager::EngineEffectsManager(EffectsResponsePipe* pResponsePipe)
        : m_pResponsePipe(pResponsePipe) {
}

EngineEffectsManager::~EngineEffectsManager() {
}

void EngineEffectsManager::onCallbackStart() {
    EffectsRequest request;
    while (m_pResponsePipe->readMessages(&request, 1) > 0) {
        EffectsResponse response(request);
        bool processed = false;
        switch (request.type) {
            case EffectsRequest::ADD_EFFECT_CHAIN:
            case EffectsRequest::REMOVE_EFFECT_CHAIN:
            case EffectsRequest::ADD_EFFECT_CHAIN_SLOT:
            case EffectsRequest::REMOVE_EFFECT_CHAIN_SLOT:
                if (processEffectsRequest(request, m_pResponsePipe.data())) {
                    processed = true;
                }
                break;
            case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
                foreach (EngineEffectChain* pChain, m_chains) {
                    if (pChain->processEffectsRequest(request, m_pResponsePipe.data())) {
                        processed = true;
                        break;
                    }
                }
                if (!processed) {
                    // If we got here, the message was not handled.
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_CHAIN;
                }
                break;

            default:
                response.success = false;
                response.status = EffectsResponse::UNHANDLED_MESSAGE_TYPE;
                break;
        }

        if (!processed) {
            m_pResponsePipe->writeMessages(&response, 1);
        }
    }
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
                                                 EffectsResponsePipe* pResponsePipe) {
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
