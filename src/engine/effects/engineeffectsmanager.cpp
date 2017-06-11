#include "engine/effects/engineeffectsmanager.h"

#include "engine/effects/engineeffectrack.h"
#include "engine/effects/engineeffectchain.h"
#include "engine/effects/engineeffect.h"

#include "util/defs.h"
#include "util/sample.h"

EngineEffectsManager::EngineEffectsManager(EffectsResponsePipe* pResponsePipe)
        : m_pResponsePipe(pResponsePipe),
          m_buffer1(MAX_BUFFER_LEN),
          m_buffer2(MAX_BUFFER_LEN) {
    // Try to prevent memory allocation.
    m_preFaderRacks.reserve(256);
    m_postFaderRacks.reserve(256);
    m_chains.reserve(256);
    m_effects.reserve(256);
}

EngineEffectsManager::~EngineEffectsManager() {
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
                if (!m_preFaderRacks.contains(request->pTargetRack)
                    && !m_postFaderRacks.contains(request->pTargetRack)) {
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
            case EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_CHANNEL:
            case EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_CHANNEL:
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

void EngineEffectsManager::processPreFader(const ChannelHandle& handle,
                                           CSAMPLE* pInOut,
                                           const unsigned int numSamples,
                                           const unsigned int sampleRate) {
    // Feature state is gathered after prefader effects processing.
    // This is okay because the equalizer and filter effects do not make use of it.
    // However, if an effect is loaded into a QuickEffectRack that could make use
    // of the GroupFeatureState, it will not sound the same as if it is loaded into
    // a StandardEffectRack.
    GroupFeatureState featureState;
    processInner(m_preFaderRacks,
                 handle, handle,
                 pInOut, pInOut,
                 numSamples, sampleRate, featureState);
}

void EngineEffectsManager::processPostFader(const ChannelHandle& inputHandle,
                                   const ChannelHandle& outputHandle,
                                   CSAMPLE* pIn, CSAMPLE* pOut,
                                   const unsigned int numSamples,
                                   const unsigned int sampleRate,
                                   const GroupFeatureState& groupFeatures) {
    processInner(m_postFaderRacks,
                inputHandle, outputHandle,
                pIn, pOut,
                numSamples, sampleRate, groupFeatures);
}

void EngineEffectsManager::processInner(const QList<EngineEffectRack*>& racks,
                                        const ChannelHandle& inputHandle,
                                        const ChannelHandle& outputHandle,
                                        CSAMPLE* pIn, CSAMPLE* pOut,
                                        const unsigned int numSamples,
                                        const unsigned int sampleRate,
                                        const GroupFeatureState& groupFeatures) {
    int racksProcessed = 0;
    CSAMPLE* pIntermediateInput = pIn;
    CSAMPLE* pIntermediateOutput = m_buffer1.data();

    for (EngineEffectRack* pRack : racks) {
        if (pRack != nullptr) {
            if (pRack->process(inputHandle, outputHandle,
                               pIntermediateInput, pIntermediateOutput,
                               numSamples, sampleRate, groupFeatures)) {
                ++racksProcessed;
                if (racksProcessed % 2) {
                    pIntermediateInput = m_buffer1.data();
                    pIntermediateOutput = m_buffer2.data();
                } else {
                    pIntermediateInput = m_buffer2.data();
                    pIntermediateOutput = m_buffer1.data();
                }
            }
        }
    }

    if (pIn != pOut) {
        // Mix effects output into output buffer.
        // ChannelMixer::applyEffectsAndMixChannels(Ramping) use this to mix channels
        // regardless of whether any effects were processsed.
        SampleUtil::copy2WithGain(pOut,
                                  pOut, 1.0,
                                  pIntermediateInput, 1.0,
                                  numSamples);
    } else if (pIn == pOut && racksProcessed > 0) {
        // Replace output buffer with effects output. If no effects were processed,
        // nothing needs to be done.
        SampleUtil::copy(pOut, pIntermediateInput, numSamples);
    }
}

bool EngineEffectsManager::addPostFaderEffectRack(EngineEffectRack* pRack) {
    if (m_postFaderRacks.contains(pRack)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: EffectRack already added to EngineEffectsManager:"
                     << pRack->number();
        }
        return false;
    }
    m_postFaderRacks.append(pRack);
    return true;
}

bool EngineEffectsManager::removePostFaderEffectRack(EngineEffectRack* pRack) {
    return m_postFaderRacks.removeAll(pRack) > 0;
}

bool EngineEffectsManager::addPreFaderEffectRack(EngineEffectRack* pRack) {
    if (m_preFaderRacks.contains(pRack)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: EffectRack already added to EngineEffectsManager:"
                     << pRack->number();
        }
        return false;
    }
    m_preFaderRacks.append(pRack);
    return true;
}

bool EngineEffectsManager::removePreFaderEffectRack(EngineEffectRack* pRack) {
    return m_preFaderRacks.removeAll(pRack) > 0;
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
            if (message.AddEffectRack.preFader) {
                response.success = addPreFaderEffectRack(message.AddEffectRack.pRack);
            } else {
                response.success = addPostFaderEffectRack(message.AddEffectRack.pRack);
            }
            break;
        case EffectsRequest::REMOVE_EFFECT_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "REMOVE_EFFECT_RACK"
                         << message.RemoveEffectRack.pRack;
            }
            if (message.AddEffectRack.preFader) {
                response.success = removePreFaderEffectRack(message.AddEffectRack.pRack);
            } else {
                response.success = removePostFaderEffectRack(message.AddEffectRack.pRack);
            }
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}
