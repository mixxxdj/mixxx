
#include "effects/effectchainmanager.h"

#include "effects/effectsmanager.h"

EffectChainManager::EffectChainManager(EffectsManager* pEffectsManager)
        : QObject(pEffectsManager),
          m_pEffectsManager(pEffectsManager) {
}

EffectChainManager::~EffectChainManager() {
    qDebug() << debugString() << "destroyed";
}

void EffectChainManager::registerGroup(const QString& group) {
    if (m_registeredGroups.contains(group)) {
        qDebug() << debugString() << "WARNING: Group already registered:"
                 << group;
        return;
    }
    m_registeredGroups.insert(group);

    foreach (EffectRackPointer pRack, m_effectRacks) {
        pRack->registerGroup(group);
    }
}

EffectRackPointer EffectChainManager::addEffectRack() {
    EffectRackPointer pRack = EffectRackPointer(new EffectRack(
        this, m_effectRacks.size()));
    m_effectRacks.append(pRack);
    return pRack;
}

EffectRackPointer EffectChainManager::getEffectRack(int i) {
    if (i < 0 || i >= m_effectRacks.size()) {
        return EffectRackPointer();
    }
    return m_effectRacks[i];
}

void EffectChainManager::addEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_effectChains.append(pEffectChain);
        pEffectChain->addToEngine();
    }
}

void EffectChainManager::removeEffectChain(EffectChainPointer pEffectChain) {
    if (pEffectChain) {
        m_effectChains.removeAll(pEffectChain);
        pEffectChain->removeFromEngine();
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
