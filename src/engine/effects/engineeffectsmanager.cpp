#include "engine/effects/engineeffectsmanager.h"

#include "engine/effects/engineeffectrack.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffect.h"

EngineEffectsManager::EngineEffectsManager(EffectsResponsePipe* pResponsePipe)
        : m_pResponsePipe(pResponsePipe) {
    // Try to prevent memory allocation.
    m_racks.reserve(256);
    m_chains.reserve(256);
    m_effects.reserve(256);
}

EngineEffectsManager::~EngineEffectsManager() {
    qDeleteAll(m_effects);
}

void EngineEffectsManager::onCallbackStart() {
    EffectsRequest* request = NULL;
    while (m_pResponsePipe->readMessages(&request, 1) > 0) {
        EffectsResponse response(*request);
        bool processed = false;
        switch (request->type) {
            case EffectsRequest::ADD_EFFECT_RACK:
            case EffectsRequest::REMOVE_EFFECT_RACK:
                if (processEffectsRequest(*request, m_pResponsePipe.data())) {
                    processed = true;
                }
                break;
            case EffectsRequest::ADD_CHAIN_TO_RACK:
            case EffectsRequest::REMOVE_CHAIN_FROM_RACK:
                if (!m_racks.contains(request->pTargetRack)) {
                    if (kEffectDebugOutput) {
                        qDebug() << debugString()
                                 << "WARNING: message for unloaded rack"
                                 << request->pTargetRack;
                    }
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_RACK;
                } else {
                    processed = request->pTargetRack->processEffectsRequest(
                        *request, m_pResponsePipe.data());

                    if (processed) {
                        // When an effect-chain becomes active (part of a rack), keep
                        // it in our master list so that we can respond to
                        // requests about it.
                        if (request->type == EffectsRequest::ADD_CHAIN_TO_RACK) {
                            m_chains.append(request->AddChainToRack.pChain);
                        } else if (request->type == EffectsRequest::REMOVE_CHAIN_FROM_RACK) {
                            m_chains.removeAll(request->RemoveChainFromRack.pChain);
                        }
                    } else {
                        if (!processed) {
                            // If we got here, the message was not handled for
                            // an unknown reason.
                            response.success = false;
                            response.status = EffectsResponse::INVALID_REQUEST;
                        }
                    }
                }
                break;
            case EffectsRequest::ADD_EFFECT_TO_CHAIN:
            case EffectsRequest::REMOVE_EFFECT_FROM_CHAIN:
            case EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS:
            case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_GROUP:
            case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_GROUP:
                if (!m_chains.contains(request->pTargetChain)) {
                    if (kEffectDebugOutput) {
                        qDebug() << debugString()
                                 << "WARNING: message for unloaded chain"
                                 << request->pTargetChain;
                    }
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_CHAIN;
                } else {
                    processed = request->pTargetChain->processEffectsRequest(
                        *request, m_pResponsePipe.data());

                    if (processed) {
                        // When an effect becomes active (part of a chain), keep
                        // it in our master list so that we can respond to
                        // requests about it.
                        if (request->type == EffectsRequest::ADD_EFFECT_TO_CHAIN) {
                            m_effects.append(request->AddEffectToChain.pEffect);
                        } else if (request->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
                            m_effects.removeAll(request->RemoveEffectFromChain.pEffect);
                        }
                    } else {
                        if (!processed) {
                            // If we got here, the message was not handled for
                            // an unknown reason.
                            response.success = false;
                            response.status = EffectsResponse::INVALID_REQUEST;
                        }
                    }
                }
                break;
            case EffectsRequest::SET_EFFECT_PARAMETERS:
            case EffectsRequest::SET_PARAMETER_PARAMETERS:
                if (!m_effects.contains(request->pTargetEffect)) {
                    if (kEffectDebugOutput) {
                        qDebug() << debugString()
                                 << "WARNING: message for unloaded effect"
                                 << request->pTargetEffect;
                    }
                    response.success = false;
                    response.status = EffectsResponse::NO_SUCH_EFFECT;
                } else {
                    processed = request->pTargetEffect
                            ->processEffectsRequest(*request, m_pResponsePipe.data());

                    if (!processed) {
                        // If we got here, the message was not handled for an
                        // unknown reason.
                        response.success = false;
                        response.status = EffectsResponse::INVALID_REQUEST;
                    }
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
                                   CSAMPLE* pInOut,
                                   const unsigned int numSamples,
                                   const unsigned int sampleRate,
                                   const GroupFeatureState& groupFeatures) {
    foreach (EngineEffectRack* pRack, m_racks) {
        pRack->process(group, pInOut, numSamples, sampleRate, groupFeatures);
    }
}

bool EngineEffectsManager::addEffectRack(EngineEffectRack* pRack) {
    if (m_racks.contains(pRack)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: EffectRack already added to EngineEffectsManager:"
                     << pRack->number();
        }
        return false;
    }
    m_racks.append(pRack);
    return true;
}

bool EngineEffectsManager::removeEffectRack(EngineEffectRack* pRack) {
    return m_racks.removeAll(pRack) > 0;
}

bool EngineEffectsManager::processEffectsRequest(const EffectsRequest& message,
                                                 EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_EFFECT_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ADD_EFFECT_RACK"
                         << message.AddEffectRack.pRack;
            }
            response.success = addEffectRack(message.AddEffectRack.pRack);
            break;
        case EffectsRequest::REMOVE_EFFECT_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "REMOVE_EFFECT_RACK"
                         << message.RemoveEffectRack.pRack;
            }
            response.success = removeEffectRack(message.RemoveEffectRack.pRack);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}
