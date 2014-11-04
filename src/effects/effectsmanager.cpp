#include "effects/effectsmanager.h"

#include <QMetaType>
#include <QtAlgorithms>

#include "engine/effects/engineeffectsmanager.h"
#include "controlobjectthread.h"
#include "controlobjectslave.h"

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

    delete m_pHiEqFreq;
    delete m_pLoEqFreq;
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

const QSet<QPair<QString, QString> > EffectsManager::getAvailableEffectNames() const {
    QSet<QPair<QString, QString> > availableEffectNames;
    QString currentEffectName;
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        QSet<QString> backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            currentEffectName = pBackend->getManifest(effectId).name();
            if (availableEffectNames.contains(qMakePair(effectId, currentEffectName))) {
                qWarning() << "WARNING: Duplicate effect name" << currentEffectName;
                continue;
            }
            availableEffectNames.insert(qMakePair(effectId, currentEffectName));
        }
    }
    return availableEffectNames;
}

const QSet<QPair<QString, QString> > EffectsManager::getAvailableMixingEqEffectNames() const {
    QSet<QPair<QString, QString> > availableEQEffectNames;
    QString currentEffectName;
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        QSet<QString> backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            if (pBackend->getManifest(effectId).isMixingEQ()) {
                currentEffectName = pBackend->getManifest(effectId).name();
                if (availableEQEffectNames.contains(qMakePair(effectId, currentEffectName))) {
                    qWarning() << "WARNING: Duplicate effect name" << currentEffectName;
                    continue;
                }
                availableEQEffectNames.insert(qMakePair(effectId, currentEffectName));
            }
        }
    }
    return availableEQEffectNames;
}

const QSet<QPair<QString, QString> > EffectsManager::getAvailableFilterEffectNames() const {
    QSet<QPair<QString, QString> > availableFilterEffectNames;
    QString currentEffectName;
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        QSet<QString> backendEffects = pBackend->getEffectIds();
        foreach (QString effectId, backendEffects) {
            if (pBackend->getManifest(effectId).isForFilterKnob()) {
                currentEffectName = pBackend->getManifest(effectId).name();
                if (availableFilterEffectNames.contains(qMakePair(effectId, currentEffectName))) {
                    qWarning() << "WARNING: Duplicate effect name" << currentEffectName;
                    continue;
                }
                availableFilterEffectNames.insert(qMakePair(effectId, currentEffectName));
            }
        }
    }
    return availableFilterEffectNames;
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

    --it;
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

EffectRackPointer EffectsManager::getEQEffectRack() {
    // The EQ Rack is the last one
    int eqRackNumber = m_pEffectChainManager->getEffectRacksSize();
    return m_pEffectChainManager->getEffectRack(eqRackNumber - 1);
}

int EffectsManager::getEQEffectRackNumber() {
    // The EQ Rack is the last one
    int eqRackNumber = m_pEffectChainManager->getEffectRacksSize();
    return eqRackNumber;
}

void EffectsManager::addEqualizer(int channelNumber) {
    int rackNum = getEffectChainManager()->getEffectRacksSize();
    EffectRackPointer pRack = getEffectRack(rackNum - 1);
    pRack->addEffectChainSlotForEQ();

    // Set the EQ to be active on Deck 'channelNumber'
    ControlObject::set(ConfigKey(QString("[EffectRack%1_EffectUnit%2]").
                arg(rackNum).arg(channelNumber),
                QString("group_[Channel%1]_enable").arg(channelNumber)),
            1.0);

    // Set the EQ to be fully wet
    ControlObject::set(ConfigKey(QString("[EffectRack%1_EffectUnit%2]").
                arg(rackNum).arg(channelNumber),
                QString("mix")),
            1.0);

    // Create aliases
    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterLow"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect1]").
                                  arg(rackNum).arg(channelNumber), "parameter1"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterMid"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect1]").
                                  arg(rackNum).arg(channelNumber), "parameter2"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterHigh"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect1]").
                                  arg(rackNum).arg(channelNumber), "parameter3"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterLowKill"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect1]").
                                  arg(rackNum).arg(channelNumber), "button_parameter1"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterMidKill"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect1]").
                                  arg(rackNum).arg(channelNumber), "button_parameter2"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterHighKill"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect1]").
                                  arg(rackNum).arg(channelNumber), "button_parameter3"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filterDepth"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2]").
                                  arg(rackNum).arg(channelNumber), "parameter"));

    ControlDoublePrivate::insertAlias(
                ConfigKey(QString("[Channel%1]").arg(channelNumber), "filter"),
                ConfigKey(QString("[EffectRack%1_EffectUnit%2_Effect2]").
                                  arg(rackNum).arg(channelNumber), "enabled"));
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
            }

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }

}
