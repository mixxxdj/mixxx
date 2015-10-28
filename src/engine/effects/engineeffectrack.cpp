#include "engine/effects/engineeffectrack.h"

#include "engine/effects/engineeffectchain.h"

EngineEffectRack::EngineEffectRack(int iRackNumber)
        : m_iRackNumber(iRackNumber) {
    // Try to prevent memory allocation.
    m_chains.reserve(256);
}

EngineEffectRack::~EngineEffectRack() {
    //qDebug() << "EngineEffectRack::~EngineEffectRack()" << this;
}

bool EngineEffectRack::processEffectsRequest(const EffectsRequest& message,
                                             EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_CHAIN_TO_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "ADD_CHAIN_TO_RACK"
                         << message.AddChainToRack.pChain
                         << message.AddChainToRack.iIndex;
            }
            response.success = addEffectChain(message.AddChainToRack.pChain,
                                              message.AddChainToRack.iIndex);
            break;
        case EffectsRequest::REMOVE_CHAIN_FROM_RACK:
            if (kEffectDebugOutput) {
                qDebug() << debugString() << "REMOVE_CHAIN_FROM_RACK"
                         << message.RemoveChainFromRack.pChain
                         << message.RemoveChainFromRack.iIndex;
            }
            response.success = removeEffectChain(message.RemoveChainFromRack.pChain,
                                                 message.RemoveChainFromRack.iIndex);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

void EngineEffectRack::process(const ChannelHandle& handle,
                               CSAMPLE* pInOut,
                               const unsigned int numSamples,
                               const unsigned int sampleRate,
                               const GroupFeatureState& groupFeatures) {
    foreach (EngineEffectChain* pChain, m_chains) {
        if (pChain != NULL) {
            pChain->process(handle, pInOut, numSamples, sampleRate, groupFeatures);
        }
    }
}

bool EngineEffectRack::addEffectChain(EngineEffectChain* pChain, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: ADD_CHAIN_TO_RACK message with invalid index:"
                     << iIndex;
        }
        return false;
    }
    if (m_chains.contains(pChain)) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "WARNING: chain already added to EngineEffectRack:"
                     << pChain->id();
        }
        return false;
    }
    while (iIndex >= m_chains.size()) {
        m_chains.append(NULL);
    }
    m_chains.replace(iIndex, pChain);
    return true;
}

bool EngineEffectRack::removeEffectChain(EngineEffectChain* pChain, int iIndex) {
    if (iIndex < 0) {
        if (kEffectDebugOutput) {
            qDebug() << debugString()
                     << "WARNING: REMOVE_CHAIN_FROM_RACK message with invalid index:"
                     << iIndex;
        }
        return false;
    }

    if (m_chains.at(iIndex) != pChain) {
        qDebug() << debugString()
                 << "WARNING: REMOVE_CHAIN_FROM_RACK consistency error"
                 << m_chains.at(iIndex) << "loaded but received request to remove"
                 << pChain;
        return false;
    }

    m_chains.replace(iIndex, NULL);
    return true;
}
