
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
    }
}

EffectChainPointer EffectChainManager::getNextEffectChain(EffectChainPointer pEffectChain) {
    int indexOf = m_effectChains.lastIndexOf(pEffectChain);

    if (indexOf == -1) {
        qDebug() << debugString() << "WARNING: getNextEffectChain called for an unmanaged EffectChain";
        return EffectChainPointer();
    }

    return m_effectChains[(indexOf + 1) % m_effectChains.size()];
}

EffectChainPointer EffectChainManager::getPrevEffectChain(EffectChainPointer pEffectChain) {
    int indexOf = m_effectChains.lastIndexOf(pEffectChain);

    if (indexOf == -1) {
        qDebug() << debugString() << "WARNING: getPrevEffectChain called for an unmanaged EffectChain";
        return EffectChainPointer();
    }

    return m_effectChains[(indexOf - 1) % m_effectChains.size()];
}

void EffectChainManager::saveEffectChains() {
    qDebug() << debugString() << "saveEffectChains";
}

void EffectChainManager::loadEffectChains() {
    qDebug() << debugString() << "loadEffectChains";
}
