#include "effects/effectchain.h"
#include "effects/effectsmanager.h"
#include "engine/effects/message.h"
#include "engine/effects/engineeffectchain.h"
#include "sampleutil.h"

EffectChain::EffectChain(EffectsManager* pEffectsManager, const QString& id)
        : QObject(pEffectsManager),
          m_pEffectsManager(pEffectsManager),
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

QString EffectChain::id() const {
    return m_id;
}

QString EffectChain::name() const {
    return m_name;
}

void EffectChain::setName(const QString& name) {
    m_name = name;
}

bool EffectChain::enabled() const {
    return m_bEnabled;
}

void EffectChain::setEnabled(bool enabled) {
    m_bEnabled = enabled;
    sendParameterUpdate();
}

void EffectChain::enableForGroup(const QString& group) {
    if (!m_enabledGroups.contains(group)) {
        m_enabledGroups.insert(group);

        EffectsRequest* request = new EffectsRequest();
        request->type = EffectsRequest::ENABLE_EFFECT_CHAIN_FOR_GROUP;
        request->pTargetChain = m_pEngineEffectChain;
        request->group = group;
        m_pEffectsManager->writeRequest(request);
    }
}

bool EffectChain::enabledForGroup(const QString& group) const {
    return m_enabledGroups.contains(group);
}

void EffectChain::disableForGroup(const QString& group) {
    if (m_enabledGroups.remove(group)) {
        EffectsRequest* request = new EffectsRequest();
        request->type = EffectsRequest::DISABLE_EFFECT_CHAIN_FOR_GROUP;
        request->pTargetChain = m_pEngineEffectChain;
        request->group = group;
        m_pEffectsManager->writeRequest(request);
    }
}

double EffectChain::parameter() const {
    return m_dParameter;
}

void EffectChain::setParameter(const double& dParameter) {
    m_dParameter = dParameter;
    sendParameterUpdate();
}

double EffectChain::mix() const {
    return m_dMix;
}

void EffectChain::setMix(const double& dMix) {
    m_dMix = dMix;
    sendParameterUpdate();
}

EffectChain::InsertionType EffectChain::insertionType() const {
    return m_insertionType;
}

void EffectChain::setInsertionType(InsertionType insertionType) {
    m_insertionType = insertionType;
    sendParameterUpdate();
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
    EffectsRequest* request = new EffectsRequest();
    request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
    request->pTargetChain = m_pEngineEffectChain;
    request->AddEffectToChain.pEffect = pEffect->getEngineEffect();
    request->AddEffectToChain.iIndex = m_effects.size() - 1;
    m_pEffectsManager->writeRequest(request);
    emit(effectAdded());
}

void EffectChain::removeEffect(EffectPointer pEffect) {
    qDebug() << debugString() << "removeEffect";
    if (m_effects.removeAll(pEffect) > 0) {
        EffectsRequest* request = new EffectsRequest();
        request->type = EffectsRequest::ADD_EFFECT_TO_CHAIN;
        request->pTargetChain = m_pEngineEffectChain;
        request->RemoveEffectFromChain.pEffect = pEffect->getEngineEffect();
        m_pEffectsManager->writeRequest(request);
        emit(effectRemoved());
    }
}

unsigned int EffectChain::numEffects() const {
    return m_effects.size();
}

QList<EffectPointer> EffectChain::getEffects() const {
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
