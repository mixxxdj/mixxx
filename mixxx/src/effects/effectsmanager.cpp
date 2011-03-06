
#include "effects/effectsmanager.h"

EffectsManager::EffectsManager(QObject* pParent)
        : QObject(pParent) {
}

EffectsManager::~EffectsManager() {
    m_effectChains.clear();
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

    // Register all the existing channels with the new EffectChain
    foreach (QString channelId, m_registeredChannels) {
        pChain->registerChannel(channelId);
    }

    m_effectChains.append(EffectChainPointer(pChain));
}

EffectChainPointer EffectsManager::getEffectChain(unsigned int i) {
    QMutexLocker locker(&m_mutex);
    if (i >= m_effectChains.size()) {
        qDebug() << "WARNING: Invalid index for getEffectChain";
        return EffectChainPointer();
    }
    return m_effectChains[i];
}

void EffectsManager::process(const QString channelId,
                             const CSAMPLE* pInput, CSAMPLE* pOutput,
                             const unsigned int numSamples) {
    QMutexLocker locker(&m_mutex);

    QList<EffectChainPointer> enabledEffects;

    for (int i = 0; i < m_effectChains.size(); ++i) {
        EffectChainPointer pChain = m_effectChains[i];
        if (pChain->isEnabledForChannel(channelId))
            enabledEffects.append(pChain);
    }

    ////////////////////////////////////////////////////////////////////////////
    // AFTER THIS LINE, THE MUTEX IS UNLOCKED. DONT TOUCH ANY MEMBER STATE
    ////////////////////////////////////////////////////////////////////////////
    locker.unlock();

    bool inPlace = pInput == pOutput;
    for (int i = 0; i < enabledEffects.size(); ++i) {
        EffectChainPointer pChain = enabledEffects[i];

        if (inPlace) {
            // Since we're doing this in-place, using a temporary buffer doesn't
            // matter.
            pChain->process(channelId, pInput, pOutput, numSamples);
        } else {
            qDebug() << debugString() << "WARNING: non-inplace processing not implemented!";
            // TODO(rryan) implement. Trickier because you have to use temporary
            // memory. Punting this for now just to get everything working.
        }
    }
}

void EffectsManager::registerChannel(const QString channelId) {
    QMutexLocker locker(&m_mutex);
    if (m_registeredChannels.contains(channelId)) {
        qDebug() << debugString() << "WARNING: Channel already registered:" << channelId;
        return;
    }

    m_registeredChannels.insert(channelId);
    foreach (EffectChainPointer effectChain, m_effectChains) {
        effectChain->registerChannel(channelId);
    }
}
