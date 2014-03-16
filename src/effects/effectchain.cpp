#include "effects/effectchain.h"
#include "effects/effectsmanager.h"
#include "effects/effectchainmanager.h"
#include "engine/effects/message.h"
#include "engine/effects/engineeffectchain.h"
#include "sampleutil.h"
#include "xmlparse.h"

EffectChain::EffectChain(EffectsManager* pEffectsManager, const QString& id,
                         EffectChainPointer pPrototype)
        : QObject(pEffectsManager),
          m_pEffectsManager(pEffectsManager),
          m_pPrototype(pPrototype),
          m_bEnabled(false),
          m_id(id),
          m_name(""),
          m_insertionType(EffectChain::INSERT),
          m_dMix(0),
          m_dParameter(0),
          m_pEngineEffectChain(new EngineEffectChain(m_id)) {
}

EffectChain::~EffectChain() {
    qDebug() << debugString() << "destroyed";
}

void EffectChain::addToEngine() {
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::ADD_EFFECT_CHAIN;
    pRequest->AddEffectChain.pChain = getEngineEffectChain();
    m_pEffectsManager->writeRequest(pRequest);

    // Add all effects.
    for (int i = 0; i < m_effects.size(); ++i) {
        EffectPointer pEffect = m_effects[i];
        // Add the effect to the engine.
        addEffectToEngine(pEffect, i);
        // Update its parameters in the engine.
        pEffect->updateEngineState();
    }
}

void EffectChain::removeFromEngine() {
    // Order doesn't matter when removing.
    for (int i = 0; i < m_effects.size(); ++i) {
        removeEffectFromEngine(m_effects[i]);
    }

    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::REMOVE_EFFECT_CHAIN;
    pRequest->RemoveEffectChain.pChain = getEngineEffectChain();
    m_pEffectsManager->writeRequest(pRequest);
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
    pClone->setParameter(pChain->parameter());
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

double EffectChain::parameter() const {
    return m_dParameter;
}

void EffectChain::setParameter(const double& dParameter) {
    m_dParameter = dParameter;
    sendParameterUpdate();
    emit(parameterChanged(dParameter));
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
    qDebug() << debugString() << "addEffect";
    if (m_effects.contains(pEffect)) {
        qDebug() << debugString()
                 << "WARNING: EffectChain already contains Effect:"
                 << pEffect;
        return;
    }
    m_effects.append(pEffect);
    addEffectToEngine(pEffect, m_effects.size() - 1);
    emit(effectAdded());
}

void EffectChain::removeEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "removeEffect";
    if (m_effects.removeAll(pEffect) > 0) {
        removeEffectFromEngine(pEffect);
        emit(effectRemoved());
    }
}

unsigned int EffectChain::numEffects() const {
    return m_effects.size();
}

const QList<EffectPointer>& EffectChain::effects() const {
    return m_effects;
}

EffectPointer EffectChain::getEffect(unsigned int effectNumber) const {
    if (effectNumber >= m_effects.size()) {
        qDebug() << debugString() << "WARNING: list index out of bounds for getEffect";
    }
    return m_effects[effectNumber];
}

EngineEffectChain* EffectChain::getEngineEffectChain() {
    return m_pEngineEffectChain;
}

void EffectChain::sendParameterUpdate() {
    EffectsRequest* pRequest = new EffectsRequest();
    pRequest->type = EffectsRequest::SET_EFFECT_CHAIN_PARAMETERS;
    pRequest->pTargetChain = m_pEngineEffectChain;
    pRequest->SetEffectChainParameters.enabled = m_bEnabled;
    pRequest->SetEffectChainParameters.insertion_type = m_insertionType;
    pRequest->SetEffectChainParameters.mix = m_dMix;
    pRequest->SetEffectChainParameters.parameter = m_dParameter;
    m_pEffectsManager->writeRequest(pRequest);
}

void EffectChain::addEffectToEngine(EffectPointer pEffect, int iIndex) {
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->AddEffectToChain.pEffect = pEffect->getEngineEffect();
    request->AddEffectToChain.iIndex = iIndex;
    m_pEffectsManager->writeRequest(request);
}

void EffectChain::removeEffectFromEngine(EffectPointer pEffect) {
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::REMOVE_EFFECT_FROM_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->RemoveEffectFromChain.pEffect = pEffect->getEngineEffect();
    m_pEffectsManager->writeRequest(request);
}

QDomElement EffectChain::toXML(QDomDocument* doc) const {
    QDomElement element = doc->createElement("EffectChain");

    XmlParse::addElement(*doc, element, "Id", m_id);
    XmlParse::addElement(*doc, element, "Name", m_name);
    XmlParse::addElement(*doc, element, "Description", m_description);
    XmlParse::addElement(*doc, element, "InsertionType",
                         insertionTypeToString(m_insertionType));
    XmlParse::addElement(*doc, element, "Mix",
                         QString::number(m_dMix));
    XmlParse::addElement(*doc, element, "Parameter",
                         QString::number(m_dParameter));

    QDomElement effectsNode = doc->createElement("Effects");
    foreach (EffectPointer pEffect, m_effects) {
        QDomElement effectNode = pEffect->toXML(doc);
        effectsNode.appendChild(effectNode);
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
    QString mixStr = XmlParse::selectNodeQString(element, "Mix");
    QString parameterStr = XmlParse::selectNodeQString(element, "ParameterStr");

    EffectChain* pChain = new EffectChain(pEffectsManager, id);
    pChain->setName(name);
    pChain->setDescription(description);
    InsertionType insertionType = insertionTypeFromString(insertionTypeStr);
    if (insertionType != NUM_INSERTION_TYPES) {
        pChain->setInsertionType(insertionType);
    }
    bool ok = false;
    double mix = mixStr.toDouble(&ok);
    if (ok) {
        pChain->setMix(mix);
    }

    ok = false;
    double parameter = parameterStr.toDouble(&ok);
    if (ok) {
        pChain->setParameter(parameter);
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
