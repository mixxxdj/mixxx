
#include "effects/effectchainmanager.h"

#include "effects/effectsmanager.h"

EffectChainManager::EffectChainManager(EffectsManager* pEffectsManager)
        : QObject(pEffectsManager),
          m_pEffectsManager(pEffectsManager) {
}

EffectChainManager::~EffectChainManager() {
    qDebug() << debugString() << "destroyed";
}

void EffectChainManager::addEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_effectChains.append(pEffectChain);
        EffectsRequest* pRequest = new EffectsRequest();
        pRequest->type = EffectsRequest::ADD_EFFECT_CHAIN;
        pRequest->AddEffectChain.pChain = pEffectChain->getEngineEffectChain();
        m_pEffectsManager->writeRequest(pRequest);

    }
}

EffectChainPointer EffectChainManager::getNextEffectChain(EffectChainPointer pEffectChain) {
    if (m_effectChains.size() == 0)
        return EffectChainPointer();

    if (!pEffectChain) {
        return m_effectChains[0];
    }

    int indexOf = m_effectChains.lastIndexOf(pEffectChain);
    if (indexOf == -1) {
        qDebug() << debugString() << "WARNING: getNextEffectChain called for an unmanaged EffectChain";
        return m_effectChains[0];
    }

    return m_effectChains[(indexOf + 1) % m_effectChains.size()];
}

EffectChainPointer EffectChainManager::getPrevEffectChain(EffectChainPointer pEffectChain) {
    if (m_effectChains.size() == 0)
        return EffectChainPointer();

    if (!pEffectChain) {
        return m_effectChains[m_effectChains.size()-1];
    }

    int indexOf = m_effectChains.lastIndexOf(pEffectChain);
    if (indexOf == -1) {
        qDebug() << debugString() << "WARNING: getPrevEffectChain called for an unmanaged EffectChain";
        return m_effectChains[m_effectChains.size()-1];
    }

    return m_effectChains[(indexOf - 1 + m_effectChains.size()) % m_effectChains.size()];
}

void EffectChainManager::saveEffectChains() {
    qDebug() << debugString() << "saveEffectChains";
}

void EffectChainManager::loadEffectChains() {
    qDebug() << debugString() << "loadEffectChains";
}
