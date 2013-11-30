#include "engine/effects/engineeffectsmanager.h"

#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffect.h"

EngineEffectsManager::EngineEffectsManager(EffectsResponsePipe* pResponsePipe)
        : m_pResponsePipe(pResponsePipe) {
}

EngineEffectsManager::~EngineEffectsManager() {
}

void EngineEffectsManager::onCallbackStart() {
    EffectsRequest* request = NULL;
    while (m_pResponsePipe->readMessages(&request, 1) > 0) {
        EffectsResponse response(*request);
        bool processed = false;
        switch (request->type) {
            case EffectsRequest::ADD_EFFECT_CHAIN:
            case EffectsRequest::REMOVE_EFFECT_CHAIN:
                if (processEffectsRequest(*request, m_pResponsePipe.data())) {
                    processed = true;
                }
                break;
            case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
            case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_GROUP:
            case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_GROUP:
                foreach (EngineEffectChain* pChain, m_chains) {
                    if (pChain->processEffectsRequest(*request, m_pResponsePipe.data())) {
                        processed = true;

                        // When an effect becomes active (part of a chain), keep
                        // it in our master list so that we can respond to
                        // requests about it.
                        if (request->type == EffectsRequest::ADD_EFFECT_TO_CHAIN) {
                            m_effects.append(request->AddEffectToChain.pEffect);
                        } else if (request->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
                            m_effects.removeAll(request->RemoveEffectFromChain.pEffect);
                        }
                        break;
                    }
                }
                if (!processed) {
                    // If we got here, the message was not handled.
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_CHAIN;
                }
                break;

            case EffectsRequest::SET_EFFECT_PARAMETER:
                if (!m_effects.contains(request->SetEffectParameter.pEffect)) {
                    qDebug() << debugString()
                             << "WARNING: SetEffectParameter message for unloaded effect"
                             << request->SetEffectParameter.pEffect;
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_EFFECT;
                } else {
                    processed = request->SetEffectParameter.pEffect
                            ->processEffectsRequest(*request, m_pResponsePipe.data());

                    if (!processed) {
                        // If we got here, the message was not handled.
                        response.success = false;
                        response.status = EffectsResponse::INVALID_PARAMETER_UPDATE;
                    }
                }
                if (!processed) {
                    // If we got here, the message was not handled.
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_EFFECT;
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

void EngineEffectsManager::process(const QString& group,
                                   const CSAMPLE* pInput, CSAMPLE* pOutput,
                                   const unsigned int numSamples) {
    const bool inPlace = pInput == pOutput;
    foreach (EngineEffectChain* pChain, m_chains) {
        if (!pChain->enabledForGroup(group)) {
            continue;
        }

        if (inPlace) {
            pChain->process(group, pInput, pOutput, numSamples);
        } else {
            qDebug() << debugString() << "WARNING: non-inplace processing not implemented!";
            // TODO(rryan) implement. Trickier because you have to use temporary
            // memory. Punting this for now just to get everything working.
        }
    }
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
