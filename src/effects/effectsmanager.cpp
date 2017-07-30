#include "effects/effectsmanager.h"

#include <QMetaType>
#include <QtAlgorithms>

#include "engine/effects/engineeffectsmanager.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectrack.h"
#include "engine/effects/engineeffectchain.h"
#include "util/assert.h"

const char* kEqualizerRackName = "[EqualizerChain]";
const char* kQuickEffectRackName = "[QuickEffectChain]";

EffectsManager::EffectsManager(QObject* pParent, UserSettingsPointer pConfig)
        : QObject(pParent),
          m_pEffectChainManager(new EffectChainManager(pConfig, this)),
          m_nextRequestId(0),
          m_pLoEqFreq(NULL),
          m_pHiEqFreq(NULL),
          m_underDestruction(false) {
    qRegisterMetaType<EffectChain::InsertionType>("EffectChain::InsertionType");
    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                2048, 2048, false, false);

    m_pRequestPipe.reset(requestPipes.first);
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);

    m_pNumEffectsAvailable = new ControlObject(ConfigKey("[Master]", "num_effectsavailable"));
    m_pNumEffectsAvailable->setReadOnly();
}

EffectsManager::~EffectsManager() {
    m_underDestruction = true;
    m_pEffectChainManager->saveEffectChains();
    delete m_pEffectChainManager;
    // This must be done here, since the engineRacks are deleted via
    // the queue
    processEffectsResponses();
    while (!m_effectsBackends.isEmpty()) {
        EffectsBackend* pBackend = m_effectsBackends.takeLast();
        delete pBackend;
    }
    for (QHash<qint64, EffectsRequest*>::iterator it = m_activeRequests.begin();
         it != m_activeRequests.end();) {
        delete it.value();
        it = m_activeRequests.erase(it);
    }

    delete m_pHiEqFreq;
    delete m_pLoEqFreq;
    delete m_pNumEffectsAvailable;
    // Safe because the Engine is deleted before EffectsManager. Also, it holds
    // a bare pointer to m_pRequestPipe so it is critical that it does not
    // outlast us.
    delete m_pEngineEffectsManager;
}

bool alphabetizeEffectManifests(const EffectManifest& manifest1,
                                const EffectManifest& manifest2) {
    return QString::localeAwareCompare(manifest1.displayName(), manifest2.displayName()) < 0;
}

void EffectsManager::addEffectsBackend(EffectsBackend* pBackend) {
    VERIFY_OR_DEBUG_ASSERT(pBackend) {
        return;
    }
    m_effectsBackends.append(pBackend);

    QList<QString> backendEffects = pBackend->getEffectIds();
    for (const QString& effectId : backendEffects) {
        m_availableEffectManifests.append(pBackend->getManifest(effectId));
    }

    m_pNumEffectsAvailable->forceSet(m_availableEffectManifests.size());

    qSort(m_availableEffectManifests.begin(), m_availableEffectManifests.end(),
          alphabetizeEffectManifests);

    connect(pBackend, SIGNAL(effectRegistered(EffectManifest)),
            this, SLOT(slotBackendRegisteredEffect(EffectManifest)));

    connect(pBackend, SIGNAL(effectRegistered(EffectManifest)),
            this, SIGNAL(availableEffectsUpdated(EffectManifest)));
}

void EffectsManager::slotBackendRegisteredEffect(EffectManifest manifest) {
    auto insertion_point = qLowerBound(m_availableEffectManifests.begin(),
                                       m_availableEffectManifests.end(),
                                       manifest, alphabetizeEffectManifests);
    m_availableEffectManifests.insert(insertion_point, manifest);
    m_pNumEffectsAvailable->forceSet(m_availableEffectManifests.size());
}

void EffectsManager::registerChannel(const ChannelHandleAndGroup& handle_group) {
    m_pEffectChainManager->registerChannel(handle_group);
}

const QSet<ChannelHandleAndGroup>& EffectsManager::registeredChannels() const {
    return m_pEffectChainManager->registeredChannels();
}

const QList<EffectManifest> EffectsManager::getAvailableEffectManifestsFiltered(
        EffectManifestFilterFnc filter) const {
    if (filter == nullptr) {
        return m_availableEffectManifests;
    }

    QList<EffectManifest> list;
    for (const auto& manifest : m_availableEffectManifests) {
        if (filter(manifest)) {
            list.append(manifest);
        }
    }
    return list;
}

bool EffectsManager::isEQ(const QString& effectId) const {
    return getEffectManifest(effectId).isMixingEQ();
}

QString EffectsManager::getNextEffectId(const QString& effectId) {
    if (m_availableEffectManifests.isEmpty()) {
        return QString();
    }
    if (effectId.isNull()) {
        return m_availableEffectManifests.first().id();
    }

    int index;
    for (index = 0; index < m_availableEffectManifests.size(); ++index) {
        if (effectId == m_availableEffectManifests.at(index).id()) {
            break;
        }
    }
    if (++index >= m_availableEffectManifests.size()) {
        index = 0;
    }
    return m_availableEffectManifests.at(index).id();
}

QString EffectsManager::getPrevEffectId(const QString& effectId) {
    if (m_availableEffectManifests.isEmpty()) {
        return QString();
    }
    if (effectId.isNull()) {
        return m_availableEffectManifests.last().id();
    }

    int index;
    for (index = 0; index < m_availableEffectManifests.size(); ++index) {
        if (effectId == m_availableEffectManifests.at(index).id()) {
            break;
        }
    }
    if (--index < 0) {
        index = m_availableEffectManifests.size() - 1;
    }
    return m_availableEffectManifests.at(index).id();
}

QPair<EffectManifest, EffectsBackend*> EffectsManager::getEffectManifestAndBackend(
        const QString& effectId) const {
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return qMakePair(pBackend->getManifest(effectId), pBackend);
        }
    }

    EffectsBackend* pBackend = NULL;
    return qMakePair(EffectManifest(), pBackend);
}

EffectManifest EffectsManager::getEffectManifest(const QString& effectId) const {
    QPair<EffectManifest, EffectsBackend*> manifestAndBackend =
            getEffectManifestAndBackend(effectId);
    return manifestAndBackend.first;
}

EffectPointer EffectsManager::instantiateEffect(const QString& effectId) {
    if (effectId.isEmpty()) {
        return EffectPointer();
    }
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return pBackend->instantiateEffect(this, effectId);
        }
    }
    return EffectPointer();
}

StandardEffectRackPointer EffectsManager::addStandardEffectRack() {
    return m_pEffectChainManager->addStandardEffectRack();
}

StandardEffectRackPointer EffectsManager::getStandardEffectRack(int rack) {
    return m_pEffectChainManager->getStandardEffectRack(rack);
}

EqualizerRackPointer EffectsManager::addEqualizerRack() {
    return m_pEffectChainManager->addEqualizerRack();
}

EqualizerRackPointer EffectsManager::getEqualizerRack(int rack) {
    return m_pEffectChainManager->getEqualizerRack(rack);
}

QuickEffectRackPointer EffectsManager::addQuickEffectRack() {
    return m_pEffectChainManager->addQuickEffectRack();
}

QuickEffectRackPointer EffectsManager::getQuickEffectRack(int rack) {
    return m_pEffectChainManager->getQuickEffectRack(rack);
}

EffectRackPointer EffectsManager::getEffectRack(const QString& group) {
    return m_pEffectChainManager->getEffectRack(group);
}

void EffectsManager::setup() {
    // These controls are used inside EQ Effects
    m_pLoEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);
    m_pHiEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);

    // Add an EqualizerRack.
    EqualizerRackPointer pEqRack = addEqualizerRack();
    // Add Master EQ here, because EngineMaster is already up
    pEqRack->addEffectChainSlotForGroup("[Master]");

    // Add a QuickEffectRack
    addQuickEffectRack();

    // Add a general purpose rack
    StandardEffectRackPointer pStandardRack = addStandardEffectRack();
    for (int i = 0; i < EffectChainManager::kNumStandardEffectChains; ++i) {
        pStandardRack->addEffectChainSlot();
    }

    // populate rack and restore state from effects.xml
    m_pEffectChainManager->loadEffectChains(pStandardRack.data());

    EffectChainPointer pChain(new EffectChain(
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

    pChain = EffectChainPointer(new EffectChain(
            this, "org.mixxx.effectchain.autopan"));
    pChain->setName(tr("AutoPan"));
    pEffect = instantiateEffect("org.mixxx.effects.autopan");
    pChain->addEffect(pEffect);
    m_pEffectChainManager->addEffectChain(pChain);
}

bool EffectsManager::writeRequest(EffectsRequest* request) {
    if (m_underDestruction) {
        // Catch all delete Messages since the engine is already down
        // and we cannot what for a communication cycle
        if (request->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
            //qDebug() << debugString() << "delete" << request->RemoveEffectFromChain.pEffect;
            delete request->RemoveEffectFromChain.pEffect;
        } else if (request->type == EffectsRequest::REMOVE_CHAIN_FROM_RACK) {
            //qDebug() << debugString() << "delete" << request->RemoveEffectFromChain.pEffect;
            delete request->RemoveChainFromRack.pChain;
        } else if (request->type == EffectsRequest::REMOVE_EFFECT_RACK) {
            //qDebug() << debugString() << "delete" << request->RemoveEffectRack.pRack;
            delete request->RemoveEffectRack.pRack;
        }
        delete request;
        return false;
    }

    if (m_pRequestPipe.isNull()) {
        delete request;
        return false;
    }

    // This is effectively only GC at this point so only deal with responses
    // when writing new requests.
    processEffectsResponses();

    request->request_id = m_nextRequestId++;
    // TODO(XXX) use preallocated requests to avoid delete calls from engine
    if (m_pRequestPipe->writeMessages(&request, 1) == 1) {
        m_activeRequests[request->request_id] = request;
        return true;
    }
    delete request;
    return false;
}

void EffectsManager::processEffectsResponses() {
    if (m_pRequestPipe.isNull()) {
        return;
    }

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
            } else {
                //qDebug() << debugString() << "EffectsRequest Success"
                //           << "type" << pRequest->type;

                if (pRequest->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
                    //qDebug() << debugString() << "delete" << pRequest->RemoveEffectFromChain.pEffect;
                    delete pRequest->RemoveEffectFromChain.pEffect;
                } else if (pRequest->type == EffectsRequest::REMOVE_CHAIN_FROM_RACK) {
                    //qDebug() << debugString() << "delete" << request->RemoveEffectFromChain.pEffect;
                    delete pRequest->RemoveChainFromRack.pChain;
                } else if (pRequest->type == EffectsRequest::REMOVE_EFFECT_RACK) {
                    qDebug() << debugString() << "delete" << pRequest->RemoveEffectRack.pRack;
                    delete pRequest->RemoveEffectRack.pRack;
                }
            }

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }
}
