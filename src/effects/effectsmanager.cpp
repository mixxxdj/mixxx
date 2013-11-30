
#include "effects/effectsmanager.h"

#include "effects/effectchainmanager.h"
#include "engine/effects/engineeffectsmanager.h"

// TODO(rryan) REMOVE
#include "effects/native/flangereffect.h"

EffectsManager::EffectsManager(QObject* pParent)
        : QObject(pParent),
          m_mutex(QMutex::Recursive),
          m_pEffectChainManager(new EffectChainManager(this)) {
    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                2048, 2048, false, false);

    m_pRequestPipe.reset(requestPipes.first);
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);
}

EffectsManager::~EffectsManager() {
    m_effectChainSlots.clear();
    while (!m_effectsBackends.isEmpty()) {
        EffectsBackend* pBackend = m_effectsBackends.takeLast();
        delete pBackend;
    }
    delete m_pEffectChainManager;
}

void EffectsManager::addEffectsBackend(EffectsBackend* pBackend) {
    QMutexLocker locker(&m_mutex);
    Q_ASSERT(pBackend);
    m_effectsBackends.append(pBackend);
}

unsigned int EffectsManager::numEffectChainSlots() const {
    QMutexLocker locker(&m_mutex);
    return m_effectChainSlots.size();
}

void EffectsManager::addEffectChainSlot() {
    QMutexLocker locker(&m_mutex);
    EffectChainSlot* pChainSlot = new EffectChainSlot(this, m_effectChainSlots.size());

    // TODO(rryan) How many should we make default? They create controls that
    // the GUI may rely on, so the choice is important to communicate to skin
    // designers.
    pChainSlot->addEffectSlot();
    pChainSlot->addEffectSlot();
    pChainSlot->addEffectSlot();
    pChainSlot->addEffectSlot();

    connect(pChainSlot, SIGNAL(nextChain(const unsigned int, EffectChainPointer)),
            this, SLOT(loadNextChain(const unsigned int, EffectChainPointer)));
    connect(pChainSlot, SIGNAL(prevChain(const unsigned int, EffectChainPointer)),
            this, SLOT(loadPrevChain(const unsigned int, EffectChainPointer)));

    // Register all the existing channels with the new EffectChain
    foreach (QString channelId, m_registeredChannels) {
        pChainSlot->registerChannel(channelId);
    }

    m_effectChainSlots.append(EffectChainSlotPointer(pChainSlot));
}

EffectChainSlotPointer EffectsManager::getEffectChainSlot(unsigned int i) {
    QMutexLocker locker(&m_mutex);
    if (i >= m_effectChainSlots.size()) {
        qDebug() << "WARNING: Invalid index for getEffectChainSlot";
        return EffectChainSlotPointer();
    }
    return m_effectChainSlots[i];
}

void EffectsManager::registerChannel(const QString channelId) {
    QMutexLocker locker(&m_mutex);
    if (m_registeredChannels.contains(channelId)) {
        qDebug() << debugString() << "WARNING: Channel already registered:" << channelId;
        return;
    }

    m_registeredChannels.insert(channelId);
    foreach (EffectChainSlotPointer pEffectChainSlot, m_effectChainSlots) {
        pEffectChainSlot->registerChannel(channelId);
    }
}

void EffectsManager::loadNextChain(const unsigned int iChainSlotNumber, EffectChainPointer pLoadedChain) {
    QMutexLocker locker(&m_mutex);
    EffectChainPointer pNextChain = m_pEffectChainManager->getNextEffectChain(pLoadedChain);
    m_effectChainSlots[iChainSlotNumber]->loadEffectChain(pNextChain);
}


void EffectsManager::loadPrevChain(const unsigned int iChainSlotNumber, EffectChainPointer pLoadedChain) {
    QMutexLocker locker(&m_mutex);
    EffectChainPointer pPrevChain = m_pEffectChainManager->getPrevEffectChain(pLoadedChain);
    m_effectChainSlots[iChainSlotNumber]->loadEffectChain(pPrevChain);
}

const QSet<QString> EffectsManager::getAvailableEffects() const {
    QMutexLocker locker(&m_mutex);
    QSet<QString> availableEffects;

    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        QSet<QString> backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            if (availableEffects.contains(effectId)) {
                qDebug() << "WARNING: Duplicate effect ID" << effectId;
                continue;
            }
            availableEffects.insert(effectId);
        }
    }

    return availableEffects;
}

EffectManifest EffectsManager::getEffectManifest(const QString& effectId) const {
    QMutexLocker locker(&m_mutex);

    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return pBackend->getManifest(effectId);
        }
    }

    return EffectManifest();
}

EffectPointer EffectsManager::instantiateEffect(const QString& effectId) {
    QMutexLocker locker(&m_mutex);
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return pBackend->instantiateEffect(effectId);
        }
    }
    return EffectPointer();
}

void EffectsManager::setupDefaultChains() {
    QMutexLocker locker(&m_mutex);
    QSet<QString> effects = getAvailableEffects();

    FlangerEffect flanger;
    QString flangerId = flanger.getId();

    if (effects.contains(flangerId)) {
        EffectChainPointer pChain = EffectChainPointer(new EffectChain());
        pChain->setId("org.mixxx.effectchain.flanger");
        pChain->setName(tr("Flanger"));
        pChain->setParameter(0.0f);

        EffectPointer flanger = instantiateEffect(flangerId);
        pChain->addEffect(flanger);
        m_pEffectChainManager->addEffectChain(pChain);

        pChain = EffectChainPointer(new EffectChain());
        pChain->setId("org.mixxx.effectchain.flanger2");
        pChain->setName(tr("Flanger2"));
        pChain->setParameter(0.0f);

        flanger = instantiateEffect(flangerId);
        pChain->addEffect(flanger);
        m_pEffectChainManager->addEffectChain(pChain);
    }
}
