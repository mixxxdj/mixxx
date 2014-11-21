#include "effects/effectrack.h"

#include "effects/effectsmanager.h"
#include "effects/effectchainmanager.h"
#include "engine/effects/engineeffectrack.h"

EffectRack::EffectRack(EffectsManager* pEffectsManager,
                       EffectChainManager* pEffectChainManager,
                       const unsigned int iRackNumber,
                       const QString& group)
        : m_group(group.isEmpty() ? formatGroupString(iRackNumber) : group),
          m_pEffectsManager(pEffectsManager),
          m_pEffectChainManager(pEffectChainManager),
          m_controlNumEffectChainSlots(ConfigKey(m_group, "num_effectunits")),
          m_controlClearRack(ConfigKey(m_group, "clear")),
          m_pEngineEffectRack(new EngineEffectRack(iRackNumber)) {
    connect(&m_controlClearRack, SIGNAL(valueChanged(double)),
            this, SLOT(slotClearRack(double)));
    m_controlNumEffectChainSlots.connectValueChangeRequest(
            this, SLOT(slotNumEffectChainSlots(double)));
    addToEngine();
}

EffectRack::~EffectRack() {
    removeFromEngine();
}

EngineEffectRack* EffectRack::getEngineEffectRack() {
    return m_pEngineEffectRack;
}

void EffectRack::addToEngine() {
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::ADD_EFFECT_RACK;
    pRequest->AddEffectRack.pRack = m_pEngineEffectRack;
    m_pEffectsManager->writeRequest(pRequest);

    // Add all effect chains.
    for (int i = 0; i < m_effectChainSlots.size(); ++i) {
        EffectChainSlotPointer pSlot = m_effectChainSlots[i];
        EffectChainPointer pChain = pSlot->getEffectChain();
        if (pChain) {
            // Add the effect to the engine.
            pChain->addToEngine(m_pEngineEffectRack, i);
            // Update its parameters in the engine.
            pChain->updateEngineState();
        }
    }
}

void EffectRack::removeFromEngine() {
    // Order doesn't matter when removing.
    for (int i = 0; i < m_effectChainSlots.size(); ++i) {
        EffectChainSlotPointer pSlot = m_effectChainSlots[i];
        EffectChainPointer pChain = pSlot->getEffectChain();
        if (pChain) {
            pChain->removeFromEngine(m_pEngineEffectRack, i);
        }
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::REMOVE_EFFECT_RACK;
    pRequest->RemoveEffectRack.pRack = m_pEngineEffectRack;
    m_pEffectsManager->writeRequest(pRequest);
}

void EffectRack::registerGroup(const QString& group) {
    foreach (EffectChainSlotPointer pChainSlot, m_effectChainSlots) {
        pChainSlot->registerGroup(group);
    }
}

void EffectRack::slotNumEffectChainSlots(double v) {
    // Ignore sets to num_effectchain_slots
    Q_UNUSED(v);
    //qDebug() << debugString() << "slotNumEffectChainSlots" << v;
    qWarning() << "WARNING: num_effectchain_slots is a read-only control.";
}

const QString& EffectRack::getGroup() const {
    return m_group;
}

void EffectRack::loadEffectToChainSlot(const unsigned int iChainSlotNumber,
                                       const unsigned int iEffectSlotNumber,
                                       QString effectId) {
    if (iChainSlotNumber >= static_cast<unsigned int>(m_effectChainSlots.size())) {
        return;
    }
    EffectPointer pNextEffect = m_pEffectsManager->instantiateEffect(effectId);

    EffectChainSlotPointer pChainSlot = m_effectChainSlots[iChainSlotNumber];
    EffectChainPointer pChain = pChainSlot->getEffectChain();
    if (!pChain) {
        pChain = EffectChainPointer(new EffectChain(m_pEffectsManager, QString(),
                                                    EffectChainPointer()));
        pChain->setName(QObject::tr("Empty Chain"));
        pChainSlot->loadEffectChain(pChain);
    }
    pChain->replaceEffect(iEffectSlotNumber, pNextEffect);
}

void EffectRack::slotClearRack(double v) {
    if (v > 0) {
        foreach (EffectChainSlotPointer pChainSlot, m_effectChainSlots) {
            pChainSlot->clear();
        }
    }
}

int EffectRack::numEffectChainSlots() const {
    return m_effectChainSlots.size();
}


EffectChainSlotPointer EffectRack::addEffectChainSlotForEQ(QString unitGroup) {
    int iChainSlotNumber = m_effectChainSlots.size();
    if (unitGroup.isEmpty()) {
        unitGroup = QString("[EffectUnit%2]").arg(iChainSlotNumber+1);
    }
    QString group(EffectChainSlot::formatGroupString(m_group, unitGroup));
    EffectChainSlot* pChainSlot =
            new EffectChainSlot(this, group, iChainSlotNumber);

    // Add a one EffectSlot for EQDefault
    pChainSlot->addEffectSlot();

    const QSet<QString>& registeredGroups =
            m_pEffectChainManager->registeredGroups();
    foreach (const QString& group, registeredGroups) {
        pChainSlot->registerGroup(group);
    }

    EffectChainSlotPointer pChainSlotPointer = EffectChainSlotPointer(pChainSlot);
    m_effectChainSlots.append(pChainSlotPointer);
    m_controlNumEffectChainSlots.setAndConfirm(
            m_controlNumEffectChainSlots.get() + 1);

    // Now load an empty effect chain into the slot so that users can edit
    // effect slots on the fly without having to load a chain.
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager, QString(),
                                              EffectChainPointer()));
    pChain->setName(QObject::tr("Empty Chain"));
    pChainSlotPointer->loadEffectChain(pChain);

    return pChainSlotPointer;
}

EffectChainSlotPointer EffectRack::addEffectChainSlot(QString unitGroup) {
    int iChainSlotNumber = m_effectChainSlots.size();
    if (unitGroup.isEmpty()) {
        unitGroup = QString("[EffectUnit%2]").arg(iChainSlotNumber+1);
    }
    QString group(EffectChainSlot::formatGroupString(m_group, unitGroup));
    EffectChainSlot* pChainSlot =
            new EffectChainSlot(this, group, iChainSlotNumber);

    // TODO(rryan) How many should we make default? They create controls that
    // the GUI may rely on, so the choice is important to communicate to skin
    // designers.
    // TODO(rryan): This should not be done here.
    pChainSlot->addEffectSlot();
    pChainSlot->addEffectSlot();
    pChainSlot->addEffectSlot();
    pChainSlot->addEffectSlot();

    connect(pChainSlot, SIGNAL(nextChain(unsigned int, EffectChainPointer)),
            this, SLOT(loadNextChain(unsigned int, EffectChainPointer)));
    connect(pChainSlot, SIGNAL(prevChain(unsigned int, EffectChainPointer)),
            this, SLOT(loadPrevChain(unsigned int, EffectChainPointer)));

    connect(pChainSlot, SIGNAL(nextEffect(unsigned int, unsigned int, EffectPointer)),
            this, SLOT(loadNextEffect(unsigned int, unsigned int, EffectPointer)));
    connect(pChainSlot, SIGNAL(prevEffect(unsigned int, unsigned int, EffectPointer)),
            this, SLOT(loadPrevEffect(unsigned int, unsigned int, EffectPointer)));

    // Register all the existing channels with the new EffectChain
    const QSet<QString>& registeredGroups =
            m_pEffectChainManager->registeredGroups();
    foreach (const QString& group, registeredGroups) {
        pChainSlot->registerGroup(group);
    }

    EffectChainSlotPointer pChainSlotPointer = EffectChainSlotPointer(pChainSlot);
    m_effectChainSlots.append(pChainSlotPointer);
    m_controlNumEffectChainSlots.setAndConfirm(
            m_controlNumEffectChainSlots.get() + 1);

    // Now load an empty effect chain into the slot so that users can edit
    // effect slots on the fly without having to load a chain.
    EffectChainPointer pChain(new EffectChain(m_pEffectsManager, QString(),
                                              EffectChainPointer()));
    pChain->setName("Empty Chain");
    pChainSlotPointer->loadEffectChain(pChain);

    return pChainSlotPointer;
}

EffectChainSlotPointer EffectRack::getEffectChainSlot(int i) {
    if (i < 0 || i >= m_effectChainSlots.size()) {
        qWarning() << "WARNING: Invalid index for getEffectChainSlot";
        return EffectChainSlotPointer();
    }
    return m_effectChainSlots[i];
}

void EffectRack::loadNextChain(const unsigned int iChainSlotNumber,
                               EffectChainPointer pLoadedChain) {
    if (pLoadedChain) {
        pLoadedChain = pLoadedChain->prototype();
    }

    EffectChainPointer pNextChain = m_pEffectChainManager->getNextEffectChain(
            pLoadedChain);

    pNextChain = EffectChain::clone(pNextChain);
    m_effectChainSlots[iChainSlotNumber]->loadEffectChain(pNextChain);
}


void EffectRack::loadPrevChain(const unsigned int iChainSlotNumber,
                               EffectChainPointer pLoadedChain) {
    if (pLoadedChain) {
        pLoadedChain = pLoadedChain->prototype();
    }

    EffectChainPointer pPrevChain = m_pEffectChainManager->getPrevEffectChain(
        pLoadedChain);

    pPrevChain = EffectChain::clone(pPrevChain);
    m_effectChainSlots[iChainSlotNumber]->loadEffectChain(pPrevChain);
}

void EffectRack::loadNextEffect(const unsigned int iChainSlotNumber,
                                const unsigned int iEffectSlotNumber,
                                EffectPointer pEffect) {
    if (iChainSlotNumber >= static_cast<unsigned int>(m_effectChainSlots.size())) {
        return;
    }

    QString effectId = pEffect ? pEffect->getManifest().id() : QString();
    QString nextEffectId = m_pEffectsManager->getNextEffectId(effectId);
    EffectPointer pNextEffect = m_pEffectsManager->instantiateEffect(nextEffectId);

    EffectChainSlotPointer pChainSlot = m_effectChainSlots[iChainSlotNumber];
    EffectChainPointer pChain = pChainSlot->getEffectChain();
    if (!pChain) {
        pChain = EffectChainPointer(new EffectChain(m_pEffectsManager, QString(),
                                                    EffectChainPointer()));
        pChain->setName("Empty Chain");
        pChainSlot->loadEffectChain(pChain);
    }
    pChain->replaceEffect(iEffectSlotNumber, pNextEffect);
}


void EffectRack::loadPrevEffect(const unsigned int iChainSlotNumber,
                                const unsigned int iEffectSlotNumber,
                                EffectPointer pEffect) {
    if (iChainSlotNumber >= static_cast<unsigned int>(m_effectChainSlots.size())) {
        return;
    }

    QString effectId = pEffect ? pEffect->getManifest().id() : QString();
    QString prevEffectId = m_pEffectsManager->getPrevEffectId(effectId);
    EffectPointer pPrevEffect = m_pEffectsManager->instantiateEffect(prevEffectId);

    EffectChainSlotPointer pChainSlot = m_effectChainSlots[iChainSlotNumber];
    EffectChainPointer pChain = pChainSlot->getEffectChain();
    if (!pChain) {
        pChain = EffectChainPointer(new EffectChain(m_pEffectsManager, QString(),
                                                    EffectChainPointer()));
        pChain->setName("Empty Chain");
        pChainSlot->loadEffectChain(pChain);
    }

    pChain->replaceEffect(iEffectSlotNumber, pPrevEffect);
}
