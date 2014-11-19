#include "effects/effectsmanager.h"

#include <QMetaType>
#include <QtAlgorithms>

#include "engine/effects/engineeffectsmanager.h"
#include "engine/effects/engineeffect.h"

EffectsManager::EffectsManager(QObject* pParent, ConfigObject<ConfigValue>* pConfig)
        : QObject(pParent),
          m_pEffectChainManager(new EffectChainManager(pConfig, this)),
          m_nextRequestId(0),
          m_pLoEqFreq(NULL),
          m_pHiEqFreq(NULL) {
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

    delete m_pHiEqFreq;
    delete m_pLoEqFreq;
    // Safe because the Engine is deleted before EffectsManager.
 //   delete m_pEngineEffectsManager;
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

const QList<QString> EffectsManager::getAvailableEffects() const {
    QList<QString> availableEffects;

    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        const QList<QString>& backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            if (availableEffects.contains(effectId)) {
                qWarning() << "WARNING: Duplicate effect ID" << effectId;
                continue;
            }
            availableEffects.append(effectId);
        }
    }

    return availableEffects;
}

const QList<QPair<QString, QString> > EffectsManager::getEffectNamesFiltered(EffectManifestFilterFnc filter) const {
    QList<QPair<QString, QString> > filteredEQEffectNames;
    QString currentEffectName;
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        QList<QString> backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            EffectManifest manifest = pBackend->getManifest(effectId);
            if (filter && !filter(&manifest)) {
                continue;
            }
            currentEffectName = manifest.name();
            filteredEQEffectNames.append(qMakePair(effectId, currentEffectName));
        }
    }

    return filteredEQEffectNames;
}

bool EffectsManager::isEQ(const QString& effectId) const {
    return getEffectManifest(effectId).isMixingEQ();
}

QString EffectsManager::getNextEffectId(const QString& effectId) {
    const QList<QString> effects = getAvailableEffects();

    if (effects.isEmpty()) {
        return QString();
    }

    if (effectId.isNull()) {
        return effects.first();
    }

    int index = effects.indexOf(effectId);
    if (++index >= effects.size()) {
        index = 0;
    }
    return effects.at(index);
}

QString EffectsManager::getPrevEffectId(const QString& effectId) {
    const QList<QString> effects = getAvailableEffects();
    //qSort(effects.begin(), effects.end());  For alphabetical order

    if (effects.isEmpty()) {
        return QString();
    }

    if (effectId.isNull()) {
        return effects.last();
    }

    int index = effects.indexOf(effectId);
    if (--index < 0) {
        index = effects.size() - 1;
    }
    return effects.at(index);

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

EffectRackPointer EffectsManager::addEffectRack(const QString& group) {
    return m_pEffectChainManager->addEffectRack(group);
}

EffectRackPointer EffectsManager::getEffectRack(int i) {
    return m_pEffectChainManager->getEffectRack(i);
}

EffectRackPointer EffectsManager::getEQEffectRack() {
    // The EQ Rack is the last one
    int eqRackNumber = getEQEffectRackNumber();
    return m_pEffectChainManager->getEffectRack(eqRackNumber);
}

int EffectsManager::getEQEffectRackNumber() {
    // The EQ Rack is the last one
    int eqRackNumber = m_pEffectChainManager->getEffectRacksSize() - 1;
    return eqRackNumber;
}

void EffectsManager::addEqualizer(const QString& group) {
    int rackNumExt = getEQEffectRackNumber() + 1;
    EffectRackPointer pRack = getEQEffectRack();
    EffectChainSlotPointer pChainSlot = pRack->addEffectChainSlotForEQ();
    const unsigned int chainSlotNumberExt = pChainSlot->getChainSlotNumber() + 1;

    // Set the EQ to be active on group
    ControlObject::set(ConfigKey(QString("[EffectRack%1_EffectUnit%2]").arg(
            QString::number(rackNumExt),
            QString::number(chainSlotNumberExt)),
                "group_" + group + "_enable"),
            1.0);

    // Set the EQ to be fully wet
    ControlObject::set(ConfigKey(QString("[EffectRack%1_EffectUnit%2]").arg(
            QString::number(rackNumExt),
            QString::number(chainSlotNumberExt)),
                "mix"),
            1.0);

    // Create aliases
    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLow"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)), "parameter1"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMid"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)), "parameter2"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHigh"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)), "parameter3"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLowKill"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "button_parameter1"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMidKill"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "button_parameter2"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHighKill"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "button_parameter3"));
    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLow_loaded"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "parameter1_loaded"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMid_loaded"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "parameter2_loaded"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHigh_loaded"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "parameter3_loaded"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterLowKill_loaded"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "button_parameter1_loaded"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterMidKill_loaded"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "button_parameter2_loaded"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterHighKill_loaded"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect1]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "button_parameter3_loaded"));


    ControlDoublePrivate::insertAlias(ConfigKey(group, "filterDepth"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "parameter"));

    ControlDoublePrivate::insertAlias(ConfigKey(group, "filter"),
            ConfigKey(
                    QString("[EffectRack%1_EffectUnit%2_Effect2]").arg(
                            QString::number(rackNumExt),
                            QString::number(chainSlotNumberExt)),
                    "enabled"));
}

void EffectsManager::setupDefaults() {
    //m_pEffectChainManager->loadEffectChains();

    EffectRackPointer pRack = addEffectRack();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();
    pRack->addEffectChainSlot();

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

    // Add a new EffectRack for Equalizers
    addEffectRack();

    // These controls are used inside EQ Effects
    m_pLoEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);
    m_pHiEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);
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
            } else {
                //qDebug() << debugString() << "EffectsRequest Success"
                //           << "type" << pRequest->type;

                if (pRequest->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
                    //qDebug() << debugString() << "delete" << pRequest->RemoveEffectFromChain.pEffect;
                    delete pRequest->RemoveEffectFromChain.pEffect;
                }
            }

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }
}
