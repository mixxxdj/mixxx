
#include "effects/effectsmanager.h"

EffectsManager::EffectsManager(QObject* pParent)
        : QObject(pParent) {
}

EffectsManager::~EffectsManager() {
    while (!m_effectsBackends.isEmpty()) {
        EffectsBackend* pBackend = m_effectsBackends.takeLast();
        delete pBackend;
    }
}

void EffectsManager::addEffectsBackend(EffectsBackend* pBackend) {
    QMutexLocker locker(&m_mutex);
    Q_ASSERT(pBackend);
    m_effectsBackends.append(pBackend);
}

unsigned int EffectsManager::numEffectChains() const {
    QMutexLocker locker(&m_mutex);
    return m_effectChains.size();
}

void EffectsManager::addEffectChain() {
    QMutexLocker locker(&m_mutex);
    EffectChain* pChain = new EffectChain(this, m_effectChains.size());
    m_effectChains.append(EffectChainPointer(pChain, &QObject::deleteLater));
}

EffectChainPointer EffectsManager::getEffectChain(unsigned int i) {
    QMutexLocker locker(&m_mutex);
    if (i >= m_effectChains.size()) {
        qDebug() << "WARNING: Invalid index for getEffectChain";
        return EffectChainPointer();
    }
    return m_effectChains[i];
}
