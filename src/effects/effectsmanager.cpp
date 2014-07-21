#include "effects/effectsmanager.h"

#include <QMetaType>
#include <QtAlgorithms>

#include "effects/effectchainmanager.h"
#include "engine/effects/engineeffectsmanager.h"

EffectsManager::EffectsManager(QObject* pParent, ConfigObject<ConfigValue>* pConfig)
        : QObject(pParent),
          m_pEffectChainManager(new EffectChainManager(pConfig, this)),
          m_nextRequestId(0) {
    qRegisterMetaType<EffectChain::InsertionType>("EffectChain::InsertionType");
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

const QSet<QString>& EffectsManager::registeredGroups() const {
    return m_pEffectChainManager->registeredGroups();
}

const QSet<QString> EffectsManager::getAvailableEffects() const {
    QSet<QString> availableEffects;

    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        QSet<QString> backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            if (availableEffects.contains(effectId)) {
                qWarning() << "WARNING: Duplicate effect ID" << effectId;
                continue;
            }
            availableEffects.insert(effectId);
        }
    }

    return availableEffects;
}

QString EffectsManager::getNextEffectId(const QString& effectId) {
    // TODO(rryan): HACK SUPER JANK ALERT. REPLACE THIS WITH SOMETHING NOT
    // STUPID
    QList<QString> effects = getAvailableEffects().toList();
    qSort(effects.begin(), effects.end());

    if (effects.isEmpty()) {
        return QString();
    }

    if (effectId.isNull()) {
        return effects.first();
    }

    QList<QString>::const_iterator it =
            qUpperBound(effects.constBegin(), effects.constEnd(), effectId);
    if (it == effects.constEnd()) {
        return effects.first();
    }

    return *it;
}

QString EffectsManager::getPrevEffectId(const QString& effectId) {
    // TODO(rryan): HACK SUPER JANK ALERT. REPLACE THIS WITH SOMETHING NOT
    // STUPID
    QList<QString> effects = getAvailableEffects().toList();
    qSort(effects.begin(), effects.end());

    if (effects.isEmpty()) {
        return QString();
    }

    if (effectId.isNull()) {
        return effects.last();
    }

    QList<QString>::const_iterator it =
            qLowerBound(effects.constBegin(), effects.constEnd(), effectId);
    if (it == effects.constBegin()) {
        return effects.last();
    }

    it--;
    return *it;
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

EffectRackPointer EffectsManager::addEffectRack() {
    return m_pEffectChainManager->addEffectRack();
}

EffectRackPointer EffectsManager::getEffectRack(int i) {
    return m_pEffectChainManager->getEffectRack(i);
}

void EffectsManager::setupDefaults() {
    //m_pEffectChainManager->loadEffectChains();

    EffectRackPointer pRack = addEffectRack();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();

    QSet<QString> effects = getAvailableEffects();

    EffectChainPointer pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.flanger"));
    pChain->setName(tr("Flanger"));
    EffectPointer pEffect = instantiateEffect(
        "org.mixxx.effects.flanger");
    pChain->addEffect(pEffect);
    m_pEffectChainManager->addEffectChain(pChain);

    pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.bitcrusher"));
    pChain->setName(tr("BitCrusher"));
    pEffect = instantiateEffect("org.mixxx.effects.bitcrusher");
    pChain->addEffect(pEffect);
    m_pEffectChainManager->addEffectChain(pChain);

    pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.filter"));
    pChain->setName(tr("Filter"));
    pEffect = instantiateEffect("org.mixxx.effects.filter");
    pChain->addEffect(pEffect);
    m_pEffectChainManager->addEffectChain(pChain);

#ifndef __MACAPPSTORE__
    pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.reverb"));
    pChain->setName(tr("Reverb"));
    pEffect = instantiateEffect("org.mixxx.effects.reverb");
    pChain->addEffect(pEffect);
    m_pEffectChainManager->addEffectChain(pChain);
#endif

    pChain = EffectChainPointer(new EffectChain(
        this, "org.mixxx.effectchain.echo"));
    pChain->setName(tr("Echo"));
    pEffect = instantiateEffect("org.mixxx.effects.echo");
    pChain->addEffect(pEffect);
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
            qWarning() << debugString()
                       << "WARNING: EffectsResponse with an inactive request_id:"
                       << response.request_id;
        }

        while (it != m_activeRequests.end() &&
               it.key() == response.request_id) {
            EffectsRequest* pRequest = it.value();

            if (!response.success) {
                qWarning() << debugString() << "WARNING: Failed EffectsRequest"
                           << "type" << pRequest->type;
            }

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }

}
