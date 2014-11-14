#include "effects/effectchain.h"
#include "effects/effectsmanager.h"
#include "effects/effectchainmanager.h"
#include "engine/effects/message.h"
#include "engine/effects/engineeffectrack.h"
#include "engine/effects/engineeffectchain.h"
#include "sampleutil.h"
#include "xmlparse.h"

EffectChain::EffectChain(EffectsManager* pEffectsManager, const QString& id,
                         EffectChainPointer pPrototype)
        : QObject(pEffectsManager),
          m_pEffectsManager(pEffectsManager),
          m_pPrototype(pPrototype),
          m_bEnabled(true),
          m_id(id),
          m_name(""),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_pEngineEffectChain(new EngineEffectChain(m_id)),
          m_bAddedToEngine(false) {
}

EffectChain::~EffectChain() {
    // Remove all effects.
    for (int i = 0; i < m_effects.size(); ++i) {
        removeEffect(i);
    }
}

void EffectChain::addToEngine(EngineEffectRack* pRack, int iIndex) {
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::ADD_CHAIN_TO_RACK;
    pRequest->pTargetRack = pRack;
    pRequest->AddChainToRack.pChain = m_pEngineEffectChain;
    pRequest->AddChainToRack.iIndex = iIndex;
    m_pEffectsManager->writeRequest(pRequest);
    m_bAddedToEngine = true;

    // Add all effects.
    for (int i = 0; i < m_effects.size(); ++i) {
        // Add the effect to the engine.
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            pEffect->addToEngine(m_pEngineEffectChain, i);
        }
    }
}

void EffectChain::removeFromEngine(EngineEffectRack* pRack, int iIndex) {
    // Order doesn't matter when removing.
    for (int i = 0; i < m_effects.size(); ++i) {
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            pEffect->removeFromEngine(m_pEngineEffectChain, i);
        }
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::REMOVE_CHAIN_FROM_RACK;
    pRequest->pTargetRack = pRack;
    pRequest->RemoveChainFromRack.pChain = m_pEngineEffectChain;
    pRequest->RemoveChainFromRack.iIndex = iIndex;
    m_pEffectsManager->writeRequest(pRequest);
    m_bAddedToEngine = false;
}

void EffectChain::updateEngineState() {
    if (!m_bAddedToEngine) {
        return;
    }
    // Update chain parameters in the engine.
    sendParameterUpdate();
    for (int i = 0; i < m_effects.size(); ++i) {
        EffectPointer pEffect = m_effects[i];
        if (pEffect) {
            // Update effect parameters in the engine.
            pEffect->updateEngineState();
        }
    }
}

// static
EffectChainPointer EffectChain::clone(EffectChainPointer pChain) {
    if (!pChain) {
        return EffectChainPointer();
    }

    EffectChain* pClone = new EffectChain(
        pChain->m_pEffectsManager, pChain->id(), pChain);
    pClone->setEnabled(pChain->enabled());
    pClone->setName(pChain->name());
    pClone->setMix(pChain->mix());
    foreach (const QString& group, pChain->enabledGroups()) {
        pClone->enableForGroup(group);
    }
    foreach (EffectPointer pEffect, pChain->effects()) {
        EffectPointer pClonedEffect = pChain->m_pEffectsManager
                ->instantiateEffect(pEffect->getManifest().id());
        pClone->addEffect(pClonedEffect);
    }
    return EffectChainPointer(pClone);
}

EffectChainPointer EffectChain::prototype() const {
    return m_pPrototype;
}

const QString& EffectChain::id() const {
    return m_id;
}

const QString& EffectChain::name() const {
    return m_name;
}

void EffectChain::setName(const QString& name) {
    m_name = name;
    emit(nameChanged(name));
}

QString EffectChain::description() const {
    return m_description;
}

void EffectChain::setDescription(const QString& description) {
    m_description = description;
    emit(descriptionChanged(description));
}

bool EffectChain::enabled() const {
    return m_bEnabled;
}

void EffectChain::setEnabled(bool enabled) {
    m_bEnabled = enabled;
    sendParameterUpdate();
    emit(enabledChanged(enabled));
}

void EffectChain::enableForGroup(const QString& group) {
    if (!m_enabledGroups.contains(group)) {
        m_enabledGroups.insert(group);

        EffectsRequest* request = new EffectsRequest();
        request->type = EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_GROUP;
        request->pTargetChain = m_pEngineEffectChain;
        request->group = group;
        m_pEffectsManager->writeRequest(request);

        emit(groupStatusChanged(group, true));
    }
}

bool EffectChain::enabledForGroup(const QString& group) const {
    return m_enabledGroups.contains(group);
}

const QSet<QString>& EffectChain::enabledGroups() const {
    return m_enabledGroups;
}

void EffectChain::disableForGroup(const QString& group) {
    if (m_enabledGroups.remove(group)) {
        EffectsRequest* request = new EffectsRequest();
        request->type = EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_GROUP;
        request->pTargetChain = m_pEngineEffectChain;
        request->group = group;
        m_pEffectsManager->writeRequest(request);

        emit(groupStatusChanged(group, false));
    }
}

double EffectChain::mix() const {
    return m_dMix;
}

void EffectChain::setMix(const double& dMix) {
    m_dMix = dMix;
    sendParameterUpdate();
    emit(mixChanged(dMix));
}

EffectChain::InsertionType EffectChain::insertionType() const {
    return m_insertionType;
}

void EffectChain::setInsertionType(InsertionType insertionType) {
    m_insertionType = insertionType;
    sendParameterUpdate();
    emit(insertionTypeChanged(insertionType));
}

void EffectChain::addEffect(EffectPointer pEffect) {
    //qDebug() << debugString() << "addEffect";
    if (!pEffect) {
        return;
    }

    if (m_effects.contains(pEffect)) {
        qWarning() << debugString()
                 << "WARNING: EffectChain already contains Effect:"
                 << pEffect;
        return;
    }
    m_effects.append(pEffect);
    if (m_bAddedToEngine) {
        pEffect->addToEngine(m_pEngineEffectChain, m_effects.size() - 1);
    }
    emit(effectsChanged());
}

void EffectChain::replaceEffect(unsigned int effectSlotNumber,
                                EffectPointer pEffect) {
    //qDebug() << debugString() << "replaceEffect" << iEffectNumber << pEffect;
    while (effectSlotNumber >= static_cast<unsigned int>(m_effects.size())) {
        if (pEffect.isNull()) {
            return;
        }
        m_effects.append(EffectPointer());
    }


    EffectPointer pOldEffect = m_effects[effectSlotNumber];
    if (!pOldEffect.isNull()) {
        if (m_bAddedToEngine) {
            pOldEffect->removeFromEngine(m_pEngineEffectChain, effectSlotNumber);
        }
    }

    m_effects.replace(effectSlotNumber, pEffect);
    if (!pEffect.isNull()) {
        if (m_bAddedToEngine) {
            pEffect->addToEngine(m_pEngineEffectChain, effectSlotNumber);
        }
    }

    emit(effectsChanged());
}

void EffectChain::removeEffect(unsigned int effectSlotNumber) {
    replaceEffect(effectSlotNumber, EffectPointer());
}

unsigned int EffectChain::numEffects() const {
    return m_effects.size();
}

const QList<EffectPointer>& EffectChain::effects() const {
    return m_effects;
}

EngineEffectChain* EffectChain::getEngineEffectChain() {
    return m_pEngineEffectChain;
}

void EffectChain::sendParameterUpdate() {
    if (!m_bAddedToEngine) {
        return;
    }
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS;
    pRequest->pTargetChain = m_pEngineEffectChain;
    pRequest->SetEffectChainParameters.enabled = m_bEnabled;
    pRequest->SetEffectChainParameters.insertion_type = m_insertionType;
    pRequest->SetEffectChainParameters.mix = m_dMix;
    m_pEffectsManager->writeRequest(pRequest);
}

QDomElement EffectChain::toXML(QDomDocument* doc) const {
    QDomElement element = doc->createElement("EffectChain");

    XmlParse::addElement(*doc, element, "Id", m_id);
    XmlParse::addElement(*doc, element, "Name", m_name);
    XmlParse::addElement(*doc, element, "Description", m_description);
    XmlParse::addElement(*doc, element, "InsertionType",
                         insertionTypeToString(m_insertionType));

    QDomElement effectsNode = doc->createElement("Effects");
    foreach (EffectPointer pEffect, m_effects) {
        if (pEffect) {
            QDomElement effectNode = pEffect->toXML(doc);
            effectsNode.appendChild(effectNode);
        }
    }
    element.appendChild(effectsNode);

    return element;
}

// static
EffectChainPointer EffectChain::fromXML(EffectsManager* pEffectsManager,
                                        const QDomElement& element) {
    QString id = XmlParse::selectNodeQString(element, "Id");
    QString name = XmlParse::selectNodeQString(element, "Name");
    QString description = XmlParse::selectNodeQString(element, "Description");
    QString insertionTypeStr = XmlParse::selectNodeQString(element, "InsertionType");

    EffectChain* pChain = new EffectChain(pEffectsManager, id);
    pChain->setName(name);
    pChain->setDescription(description);
    InsertionType insertionType = insertionTypeFromString(insertionTypeStr);
    if (insertionType != NUM_INSERTION_TYPES) {
        pChain->setInsertionType(insertionType);
    }

    EffectChainPointer pChainWrapped(pChain);
    pEffectsManager->getEffectChainManager()->addEffectChain(pChainWrapped);

    QDomElement effects = XmlParse::selectElement(element, "Effects");
    QDomNodeList effectChildren = effects.childNodes();

    for (int i = 0; i < effectChildren.count(); ++i) {
        QDomNode effect = effectChildren.at(i);
        if (effect.isElement()) {
            EffectPointer pEffect = Effect::fromXML(
                pEffectsManager, effect.toElement());
            if (pEffect) {
                pChain->addEffect(pEffect);
            }
        }
    }

    return pChainWrapped;
}
