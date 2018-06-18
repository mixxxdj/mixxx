#include "effects/effectsmanager.h"

#include <QMetaType>
#include <QtAlgorithms>

#include <algorithm>

#include "engine/effects/engineeffectsmanager.h"
#include "effects/effectsbackend.h"
#include "effects/effectslot.h"
#include "effects/effectxmlelements.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectchain.h"
#include "util/assert.h"

namespace {
const QString kEffectGroupSeparator = "_";
const QString kGroupClose = "]";
const unsigned int kEffectMessagPipeFifoSize = 2048;
} // anonymous namespace


EffectsManager::EffectsManager(QObject* pParent, UserSettingsPointer pConfig,
                               ChannelHandleFactory* pChannelHandleFactory)
        : QObject(pParent),
          m_pChannelHandleFactory(pChannelHandleFactory),
          m_pConfig(pConfig),
          m_nextRequestId(0),
          m_pLoEqFreq(NULL),
          m_pHiEqFreq(NULL),
          m_underDestruction(false) {
    qRegisterMetaType<EffectChainMixMode>("EffectChainMixMode");
    QPair<EffectsRequestPipe*, EffectsResponsePipe*> requestPipes =
            TwoWayMessagePipe<EffectsRequest*, EffectsResponse>::makeTwoWayMessagePipe(
                kEffectMessagPipeFifoSize, kEffectMessagPipeFifoSize, false, false);

    m_pRequestPipe.reset(requestPipes.first);
    m_pEngineEffectsManager = new EngineEffectsManager(requestPipes.second);

    m_pNumEffectsAvailable = new ControlObject(ConfigKey("[Master]", "num_effectsavailable"));
    m_pNumEffectsAvailable->setReadOnly();

    // These controls are used inside EQ Effects
    m_pLoEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "LoEQFrequency"), 0., 22040);
    m_pHiEqFreq = new ControlPotmeter(ConfigKey("[Mixer Profile]", "HiEQFrequency"), 0., 22040);
}

EffectsManager::~EffectsManager() {
    m_underDestruction = true;

    // NOTE(Kshitij) : Use new functions for saving XML files
    // saveEffectChains();
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

bool alphabetizeEffectManifests(EffectManifestPointer pManifest1,
                                EffectManifestPointer pManifest2) {
    // Sort built-in effects first before external plugins
    int backendNameComparision = static_cast<int>(pManifest1->backendType()) - static_cast<int>(pManifest2->backendType());
    int displayNameComparision = QString::localeAwareCompare(pManifest1->displayName(), pManifest2->displayName());
    return (backendNameComparision ? (backendNameComparision < 0) : (displayNameComparision < 0));
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

    connect(pBackend, SIGNAL(effectRegistered(EffectManifestPointer)),
            this, SLOT(slotBackendRegisteredEffect(EffectManifestPointer)));

    connect(pBackend, SIGNAL(effectRegistered(EffectManifestPointer)),
            this, SIGNAL(availableEffectsUpdated(EffectManifestPointer)));
}

void EffectsManager::slotBackendRegisteredEffect(EffectManifestPointer pManifest) {
    auto insertion_point = std::lower_bound(m_availableEffectManifests.begin(),
                                            m_availableEffectManifests.end(),
                                            pManifest, alphabetizeEffectManifests);
    m_availableEffectManifests.insert(insertion_point, pManifest);
    m_pNumEffectsAvailable->forceSet(m_availableEffectManifests.size());
}

// TODO(Kshitij) : This function is not being used. Remove
void EffectsManager::addEffectChain(EffectChainSlotPointer pEffectChainSlot) {
    if (pEffectChainSlot) {
        m_effectChainSlots.append(pEffectChainSlot);
    }
}

// TODO(Kshitij) : This function is not being used. Remove
void EffectsManager::removeEffectChain(EffectChainSlotPointer pEffectChainSlot) {
    if (pEffectChainSlot) {
        m_effectChainSlots.removeAll(pEffectChainSlot);
    }
}

// TODO(Kshitij) : This function is not being used. Remove
EffectChainSlotPointer EffectsManager::getNextEffectChain(EffectChainSlotPointer pEffectChainSlot) {
    if (m_effectChainSlots.isEmpty())
        return EffectChainSlotPointer();

    if (!pEffectChainSlot) {
        return m_effectChainSlots[0];
    }

    int indexOf = m_effectChainSlots.lastIndexOf(pEffectChainSlot);
    if (indexOf == -1) {
        qWarning() << debugString() << "WARNING: getNextEffectChain called for an unmanaged EffectChain";
        return m_effectChainSlots[0];
    }

    return m_effectChainSlots[(indexOf + 1) % m_effectChainSlots.size()];
}

// TODO(Kshitij) : This function is not being used. Remove
EffectChainSlotPointer EffectsManager::getPrevEffectChain(EffectChainSlotPointer pEffectChainSlot) {
    if (m_effectChainSlots.isEmpty())
        return EffectChainSlotPointer();

    if (!pEffectChainSlot) {
        return m_effectChainSlots[m_effectChainSlots.size()-1];
    }

    int indexOf = m_effectChainSlots.lastIndexOf(pEffectChainSlot);
    if (indexOf == -1) {
        qWarning() << debugString() << "WARNING: getPrevEffectChain called for an unmanaged EffectChain";
        return m_effectChainSlots[m_effectChainSlots.size()-1];
    }

    return m_effectChainSlots[(indexOf - 1 + m_effectChainSlots.size()) % m_effectChainSlots.size()];
}

bool EffectsManager::isAdoptMetaknobValueEnabled() const {
    return m_pConfig->getValue(ConfigKey("[Effects]", "AdoptMetaknobValue"), true);
}

void EffectsManager::registerInputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredInputChannels.contains(handle_group)) {
        return;
    }
    m_registeredInputChannels.insert(handle_group);

    for (auto& pRack : m_standardEffectRacks) {
        pRack->registerInputChannel(handle_group);
    }
}

void EffectsManager::registerOutputChannel(const ChannelHandleAndGroup& handle_group) {
    VERIFY_OR_DEBUG_ASSERT(!m_registeredOutputChannels.contains(handle_group)) {
        return;
    }
    m_registeredOutputChannels.insert(handle_group);
}

const QList<EffectManifestPointer> EffectsManager::getAvailableEffectManifestsFiltered(
        EffectManifestFilterFnc filter) const {
    if (filter == nullptr) {
        return m_availableEffectManifests;
    }

    QList<EffectManifestPointer> list;
    for (const auto& pManifest : m_availableEffectManifests) {
        if (filter(pManifest.data())) {
            list.append(pManifest);
        }
    }
    return list;
}

bool EffectsManager::isEQ(const QString& effectId) const {
    EffectManifestPointer pManifest = getEffectManifest(effectId);
    return pManifest ? pManifest->isMixingEQ() : false;
}

QString EffectsManager::getNextEffectId(const QString& effectId) {
    if (m_availableEffectManifests.isEmpty()) {
        return QString();
    }
    if (effectId.isNull()) {
        return m_availableEffectManifests.first()->id();
    }

    int index;
    for (index = 0; index < m_availableEffectManifests.size(); ++index) {
        if (effectId == m_availableEffectManifests.at(index)->id()) {
            break;
        }
    }
    if (++index >= m_availableEffectManifests.size()) {
        index = 0;
    }
    return m_availableEffectManifests.at(index)->id();
}

QString EffectsManager::getPrevEffectId(const QString& effectId) {
    if (m_availableEffectManifests.isEmpty()) {
        return QString();
    }
    if (effectId.isNull()) {
        return m_availableEffectManifests.last()->id();
    }

    int index;
    for (index = 0; index < m_availableEffectManifests.size(); ++index) {
        if (effectId == m_availableEffectManifests.at(index)->id()) {
            break;
        }
    }
    if (--index < 0) {
        index = m_availableEffectManifests.size() - 1;
    }
    return m_availableEffectManifests.at(index)->id();
}

void EffectsManager::getEffectManifestAndBackend(
        const QString& effectId,
        EffectManifestPointer* ppManifest, EffectsBackend** ppBackend) const {
    foreach (EffectsBackend* pBackend, m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            *ppManifest = pBackend->getManifest(effectId);
            *ppBackend = pBackend;
        }
    }
}

EffectManifestPointer EffectsManager::getEffectManifest(const QString& effectId) const {
    EffectManifestPointer pMainifest;
    EffectsBackend* pEffectBackend;
    getEffectManifestAndBackend(effectId, &pMainifest, &pEffectBackend);
    return pMainifest;
}

EffectPointer EffectsManager::instantiateEffect(const QString& effectId) {
    if (effectId.isEmpty()) {
        return EffectPointer();
    }
    for (const auto& pBackend: m_effectsBackends) {
        if (pBackend->canInstantiateEffect(effectId)) {
            return pBackend->instantiateEffect(this, effectId);
        }
    }
    return EffectPointer();
}

StandardEffectRackPointer EffectsManager::addStandardEffectRack() {
    StandardEffectRackPointer pRack(new StandardEffectRack(
        this, m_standardEffectRacks.size()));
    m_standardEffectRacks.append(pRack);
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return pRack;
}

StandardEffectRackPointer EffectsManager::getStandardEffectRack(int i) {
    if (i < 0 || i >= m_standardEffectRacks.size()) {
        return StandardEffectRackPointer();
    }
    return m_standardEffectRacks[i];
}

EqualizerRackPointer EffectsManager::addEqualizerRack() {
    EqualizerRackPointer pRack(new EqualizerRack(
            this, m_equalizerEffectRacks.size()));
    m_equalizerEffectRacks.append(pRack);
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return pRack;
}

EqualizerRackPointer EffectsManager::getEqualizerRack(int i) {
    if (i < 0 || i >= m_equalizerEffectRacks.size()) {
        return EqualizerRackPointer();
    }
    return m_equalizerEffectRacks[i];
}

QuickEffectRackPointer EffectsManager::addQuickEffectRack() {
    QuickEffectRackPointer pRack(new QuickEffectRack(
        this, m_quickEffectRacks.size()));
    m_quickEffectRacks.append(pRack);
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return pRack;
}

QuickEffectRackPointer EffectsManager::getQuickEffectRack(int i) {
    if (i < 0 || i >= m_quickEffectRacks.size()) {
        return QuickEffectRackPointer();
    }
    return m_quickEffectRacks[i];
}

OutputEffectRackPointer EffectsManager::addOutputsEffectRack() {
    OutputEffectRackPointer pRack(new OutputEffectRack(this));
    m_pOutputEffectRack = pRack;
    m_effectRacksByGroup.insert(pRack->getGroup(), pRack);
    return m_pOutputEffectRack;
}

OutputEffectRackPointer EffectsManager::getOutputsEffectRack() {
    return m_pOutputEffectRack;
}

EffectRackPointer EffectsManager::getEffectRack(const QString& group) {
    return m_effectRacksByGroup.value(group);
}

EffectSlotPointer EffectsManager::getEffectSlot(
        const QString& group) {
    QRegExp intRegEx(".*(\\d+).*");

    QStringList parts = group.split(kEffectGroupSeparator);

    EffectRackPointer pRack = getEffectRack(parts.at(0) + kGroupClose);
    VERIFY_OR_DEBUG_ASSERT(pRack) {
        return EffectSlotPointer();
    }

    EffectChainSlotPointer pChainSlot;
    if (parts.at(0) == "[EffectRack1") {
        intRegEx.indexIn(parts.at(1));
        pChainSlot = pRack->getEffectChainSlot(intRegEx.cap(1).toInt() - 1);
    } else {
        // Assume a PerGroupRack
        const QString chainGroup =
                parts.at(0) + kEffectGroupSeparator + parts.at(1) + kGroupClose;
        for (int i = 0; i < pRack->numEffectChainSlots(); ++i) {
            EffectChainSlotPointer pSlot = pRack->getEffectChainSlot(i);
            if (pSlot->getGroup() == chainGroup) {
                pChainSlot = pSlot;
                break;
            }
        }
    }
    VERIFY_OR_DEBUG_ASSERT(pChainSlot) {
        return EffectSlotPointer();
    }

    intRegEx.indexIn(parts.at(2));
    EffectSlotPointer pEffectSlot =
            pChainSlot->getEffectSlot(intRegEx.cap(1).toInt() - 1);
    return pEffectSlot;
}

EffectParameterSlotPointer EffectsManager::getEffectParameterSlot(
        const ConfigKey& configKey) {
    EffectSlotPointer pEffectSlot =
             getEffectSlot(configKey.group);
    VERIFY_OR_DEBUG_ASSERT(pEffectSlot) {
        return EffectParameterSlotPointer();
    }

    QRegExp intRegEx(".*(\\d+).*");
    intRegEx.indexIn(configKey.item);
    EffectParameterSlotPointer pParameterSlot =
            pEffectSlot->getEffectParameterSlot(intRegEx.cap(1).toInt() - 1);
    return pParameterSlot;
}

EffectButtonParameterSlotPointer EffectsManager::getEffectButtonParameterSlot(
        const ConfigKey& configKey) {
    EffectSlotPointer pEffectSlot =
             getEffectSlot(configKey.group);
    VERIFY_OR_DEBUG_ASSERT(pEffectSlot) {
        return EffectButtonParameterSlotPointer();
    }

    QRegExp intRegEx(".*(\\d+).*");
    intRegEx.indexIn(configKey.item);
    EffectButtonParameterSlotPointer pParameterSlot =
            pEffectSlot->getEffectButtonParameterSlot(intRegEx.cap(1).toInt() - 1);
    return pParameterSlot;
}

void EffectsManager::setEffectVisibility(EffectManifestPointer pManifest, bool visible) {
    if (visible && !m_visibleEffectManifests.contains(pManifest)) {
        auto insertion_point = std::lower_bound(m_visibleEffectManifests.begin(),
                                                m_visibleEffectManifests.end(),
                                                pManifest, alphabetizeEffectManifests);
        m_visibleEffectManifests.insert(insertion_point, pManifest);
        emit(visibleEffectsUpdated());
    } else if (!visible) {
        m_visibleEffectManifests.removeOne(pManifest);
        emit(visibleEffectsUpdated());
    }
}

bool EffectsManager::getEffectVisibility(EffectManifestPointer pManifest) {
    return m_visibleEffectManifests.contains(pManifest);
}

// TODO: remove this when removing EffectRack layer. The EQs and QuickEffects
// should be EffectChainSlot subclasses that get initialized by PlayerManager
// with each deck
void EffectsManager::setupPerGroupRacks() {
    // NOTE(Be): Effect racks are processed in the order they are added here.
    addEqualizerRack();
    addQuickEffectRack();
}

void EffectsManager::setup() {
    // Add postfader effect racks
    addStandardEffectRack();
    addOutputsEffectRack();
}

// NOTE(Kshitij) : Use new functions for effect loading using Effect Preset
// void EffectsManager::loadEffectChains() {
//     // populate rack and restore state from effects.xml
//     m_pEffectChainManager->loadEffectChains();
// }

void EffectsManager::refreshAllRacks() {
    for (const auto& pRack: m_standardEffectRacks) {
        pRack->refresh();
    }
    for (const auto& pRack: m_equalizerEffectRacks) {
        pRack->refresh();
    }
    for (const auto& pRack: m_quickEffectRacks) {
        pRack->refresh();
    }
}

bool EffectsManager::writeRequest(EffectsRequest* request) {
    if (m_underDestruction) {
        // Catch all delete Messages since the engine is already down
        // and we cannot wait for a communication cycle
        collectGarbage(request);
    }

    if (m_pRequestPipe.isNull()) {
        delete request;
        return false;
    }

    // This is effectively only garbage collection at this point so only deal
    // with responses when writing new requests.
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

        VERIFY_OR_DEBUG_ASSERT(it != m_activeRequests.end()) {
            qWarning() << debugString()
                       << "WARNING: EffectsResponse with an inactive request_id:"
                       << response.request_id;
        }

        while (it != m_activeRequests.end() &&
               it.key() == response.request_id) {
            EffectsRequest* pRequest = it.value();

            // Don't check whether the response was successful here because
            // specific errors should be caught with DEBUG_ASSERTs in
            // EngineEffectsManager and functions it calls to handle requests.

            collectGarbage(pRequest);

            delete pRequest;
            it = m_activeRequests.erase(it);
        }
    }
}

void EffectsManager::collectGarbage(const EffectsRequest* pRequest) {
    if (pRequest->type == EffectsRequest::REMOVE_EFFECT_FROM_CHAIN) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "delete" << pRequest->RemoveEffectFromChain.pEffect;
        }
        delete pRequest->RemoveEffectFromChain.pEffect;
    } else if (pRequest->type == EffectsRequest::REMOVE_EFFECT_CHAIN) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "delete" << pRequest->RemoveEffectFromChain.pEffect;
        }
        delete pRequest->RemoveEffectChain.pChain;
    } else if (pRequest->type == EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_INPUT_CHANNEL) {
        if (kEffectDebugOutput) {
            qDebug() << debugString() << "deleting states for input channel" << pRequest->DisableInputChannelForChain.pChannelHandle << "for EngineEffectChain" << pRequest->pTargetChain;
        }
        pRequest->pTargetChain->deleteStatesForInputChannel(
                pRequest->DisableInputChannelForChain.pChannelHandle);
    }
}
