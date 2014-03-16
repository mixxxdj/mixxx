#include "engine/effects/engineeffectrack.h"

#include "engine/effects/engineeffectchain.h"

EngineEffectRack::EngineEffectRack(int iRackNumber)
        : m_iRackNumber(iRackNumber) {
    // Try to prevent memory allocation.
    m_chains.reserve(256);
}

EngineEffectRack::~EngineEffectRack() {
}

bool EngineEffectRack::processEffectsRequest(const EffectsRequest& message,
                                             EffectsResponsePipe* pResponsePipe) {
    EffectsResponse response(message);
    switch (message.type) {
        case EffectsRequest::ADD_CHAIN_TO_RACK:
            qDebug() << debugString() << "ADD_CHAIN_TO_RACK"
                     << message.AddChainToRack.pChain
                     << message.AddChainToRack.iIndex;
            response.success = addEffectChain(message.AddChainToRack.pChain,
                                              message.AddChainToRack.iIndex);
            break;
        case EffectsRequest::REMOVE_CHAIN_FROM_RACK:
            qDebug() << debugString() << "REMOVE_CHAIN_FROM_RACK"
                     << message.RemoveChainFromRack.pChain;
            response.success = removeEffectChain(
                message.RemoveChainFromRack.pChain);
            break;
        default:
            return false;
    }
    pResponsePipe->writeMessages(&response, 1);
    return true;
}

void EngineEffectRack::process(const QString& group,
                               const CSAMPLE* pInput, CSAMPLE* pOutput,
                               const unsigned int numSamples) {
    foreach (EngineEffectChain* pChain, m_chains) {
        pChain->process(group, pInput, pOutput, numSamples);
    }
}

bool EngineEffectRack::addEffectChain(EngineEffectChain* pChain, int iIndex) {
    if (iIndex < 0 || iIndex > m_chains.size()) {
        qDebug() << debugString()
                 << "WARNING: ADD_CHAIN_TO_RACK message with invalid index:"
                 << iIndex;
    }
    if (m_chains.contains(pChain)) {
        qDebug() << debugString() << "WARNING: chain already added to EngineEffectRack:"
                 << pChain->id();
        return false;
    }
    m_chains.insert(iIndex, pChain);
    return true;
}

bool EngineEffectRack::removeEffectChain(EngineEffectChain* pChain) {
    return m_chains.removeAll(pChain) > 0;
}
