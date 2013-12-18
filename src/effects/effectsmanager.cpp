#include "effects/effectsmanager.h"

#include "effects/effectchainmanager.h"
#include "engine/effects/engineeffectsmanager.h"

EffectsManager::EffectsManager(QObject* pParent, ConfigObject<ConfigValue>* pConfig)
        : QObject(pParent),
          m_pEffectChainManager(new EffectChainManager(pConfig, this)),
          m_nextRequestId(0) {
    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                2048, 2048, false, false);

    m_pRequestPipe.reset(requestPipes.first);
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);
}

EffectsManager::~EffectsManager() {
    m_pEffectChainManager->saveEffectChains();
    processEffectsResponses();
    delete m_pEffectChainManager;
    while (!m_effectsBackends.isEmpty()) {
        EffectsBackend* pBackend = m_effectsBackends.takeLast();
        delete pBackend;
    }
    for (QHash<qint64, EffectsRequest*>::iterator it = m_activeRequests.begin();
         it != m_activeRequests.end();) {
        delete it.value();
        it = m_activeRequests.erase(it);
    }
}

void EffectsManager::addEffectsBackend(EffectsBackend* pBackend) {
    Q_ASSERT(pBackend);
    m_effectsBackends.append(pBackend);
    connect(pBackend, SIGNAL(effectRegistered()),
            this, SIGNAL(availableEffectsUpdated()));
}

void EffectsManager::registerGroup(const QString& group) {
    m_pEffectChainManager->registerGroup(group);
}

const QSet<QString> EffectsManager::getAvailableEffects() const {
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
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return pBackend->getManifest(effectId);
        }
    }

    return EffectManifest();
}

EffectPointer EffectsManager::instantiateEffect(const QString& effectId) {
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return pBackend->instantiateEffect(this, effectId);
        }
    }
    return EffectPointer();
}


EffectRackPointer EffectsManager::getEffectRack(int i) {
    return m_pEffectChainManager->getEffectRack(i);
}

void EffectsManager::setupDefaults() {
    m_pEffectChainManager->loadEffectChains();

    EffectRackPointer pRack = m_pEffectChainManager->addEffectRack();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();

    QSet<QString> effects = getAvailableEffects();

    EffectChainPointer pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.flanger"));
    pChain->setName(tr("Flanger"));
    pChain->setParameter(0.0f);
    EffectPointer flanger = instantiateEffect(
        "org.mixxx.effects.flanger");
    pChain->addEffect(flanger);
    m_pEffectChainManager->addEffectChain(pChain);

    pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.bitcrusher"));
    pChain->setName(tr("BitCrusher"));
    pChain->setParameter(0.0f);
    flanger = instantiateEffect("org.mixxx.effects.bitcrusher");
    pChain->addEffect(flanger);
    m_pEffectChainManager->addEffectChain(pChain);

    pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.filter"));
    pChain->setName(tr("Filter"));
    pChain->setParameter(0.0f);
    flanger = instantiateEffect("org.mixxx.effects.filter");
    pChain->addEffect(flanger);
    m_pEffectChainManager->addEffectChain(pChain);
}

bool EffectsManager::writeRequest(EffectsRequest* request) {
    // This is effectively only GC at this point so only deal with responses
    // when writing new requests.
    processEffectsResponses();

    request->request_id = m_nextRequestId++;
    if (m_pRequestPipe->writeMessages(&request, 1) == 1) {
        m_activeRequests[request->request_id] = request;
        return true;
    }
    return false;
}

void EffectsManager::processEffectsResponses() {
    EffectsResponse response;
    while (m_pRequestPipe->readMessages(&response, 1) == 1) {
        QHash<qint64, EffectsRequest*>::iterator it =
                m_activeRequests.find(response.request_id);

        if (it == m_activeRequests.end()) {
            qDebug() << debugString()
                     << "WARNING: EffectsResponse with an inactive request_id:"
                     << response.request_id;
        }

        while (it != m_activeRequests.end() &&
               it.key() == response.request_id) {
            EffectsRequest* pRequest = it.value();

            if (!response.success) {
                qDebug() << debugString() << "WARNING: Failed EffectsRequest"
                         << "type" << pRequest->type;
            }

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }

}
